// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "audiohelper.h"
#include <QDebug>
#include <QProcess>
#include <alsa/asoundlib.h>
#include <algorithm>
#include <cstring>

AudioHelper::AudioHelper(QObject *parent) : QObject(parent) {}

void AudioHelper::forceAlsaRescan() const
{
    void **hints = nullptr;
    if (snd_device_name_hint(-1, "pcm", &hints) >= 0) {
        snd_device_name_free_hint(hints);
    }
    snd_config_update_free_global();
}

QString AudioHelper::detectMachine() const
{
    QProcess p;
    p.start("uname", QStringList() << "-n");
    if (!p.waitForStarted() || !p.waitForFinished(3000)) return "Unknown";
    QString out = p.readAllStandardOutput().trimmed();
    if (out == "sl2619-coralboard") {
        return "sl2619";
    }
    return out.isEmpty() ? "Unknown" : out;
}

QString AudioHelper::getPlatformFallbackDevice() const
{
    QString m = detectMachine();
    if (m == "sl1620") return "hw:0,1";
    if (m == "sl2619") return "hw:0,0";
    if (m == "sl2615") return "hw:0,0";
    if (m == "sl1640") return "hw:0,8";
    if (m == "sl1680") return "hw:0,7";
    return "default";
}

int AudioHelper::scorePlaybackDevice(const QString &name, const QString &id) const
{
    QString machine = detectMachine();
    QString n = name.toLower();

    if (n.contains("usb"))  return 100;
    if (machine == "sl1620" && n.contains("asoc-i2s2"))  return 80;
    if (machine == "sl2619" && n.contains("asoc-i2s1"))  return 80;
    if (machine == "sl2615" && n.contains("asoc-i2s1"))  return 80;
    if (machine == "sl1640" && n.contains("soc-hdmio"))  return 80;
    if (machine == "sl1680" && n.contains("soc-hdmio"))  return 80;

    return 10;
}

QVector<AudioHelper::DeviceInfo> AudioHelper::scanAllDevicesOnce() const
{
    forceAlsaRescan();

    QVector<DeviceInfo> devices;

    const snd_pcm_stream_t streams[] = { SND_PCM_STREAM_PLAYBACK, SND_PCM_STREAM_CAPTURE };
    const char* streamNames[] = { "Playback", "Capture" };

    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0) {
        snd_ctl_t *ctl = nullptr;
        char cardId[32];
        snprintf(cardId, sizeof(cardId), "hw:%d", card);

        if (snd_ctl_open(&ctl, cardId, 0) < 0) continue;
        int dev = -1;
        while (snd_ctl_pcm_next_device(ctl, &dev) >= 0 && dev >= 0) {
            for (int s = 0; s < 2; ++s) {
                snd_pcm_info_t *info;
                snd_pcm_info_alloca(&info);
                snd_pcm_info_set_device(info, dev);
                snd_pcm_info_set_stream(info, streams[s]);

                if (snd_ctl_pcm_info(ctl, info) < 0) continue;

                QString devId = QString("hw:%1,%2").arg(card).arg(dev);
                const char* pcmId = snd_pcm_info_get_id(info);
                QString name = (pcmId && strlen(pcmId) > 0) ? QString::fromUtf8(pcmId) : "Unknown Device";
                const char* cname = snd_pcm_info_get_name(info);
                if (!cname || strlen(cname) == 0) {
                    cname = snd_pcm_info_get_subdevice_name(info);
                }
                if (cname && strlen(cname) > 0 && name == "Unknown Device") {
                    name = QString::fromUtf8(cname);
                }

                int score = (streams[s] == SND_PCM_STREAM_PLAYBACK)
                            ? scorePlaybackDevice(name, devId) : 0;

                devices.append({devId, name, streamNames[s], score});
            }
        }
        snd_ctl_close(ctl);
    }
    return devices;
}

void AudioHelper::printDevices() const
{
    auto devices = scanAllDevicesOnce();

    qDebug() << "==================================================";
    qDebug() << "ALSA Audio Devices (Live Hotplug Detection)";
    qDebug() << "Machine:" << detectMachine();
    qDebug() << "";

    QString bestId;
    int bestScore = -1;
    QString bestName;

    qDebug() << "PLAYBACK DEVICES:";
    for (const auto &d : devices) {
        if (d.type == "Playback") {
            qDebug().noquote() << QString("  [%1] %2 → %3")
                                  .arg(d.score, 3).arg(d.id, -12).arg(d.name);
            if (d.score > bestScore) {
                bestScore = d.score;
                bestId = d.id;
                bestName = d.name;
            }
        }
    }

    qDebug() << "\nCAPTURE DEVICES:";
    for (const auto &d : devices) {
        if (d.type == "Capture") {
            qDebug().noquote() << QString("      %1 → %2").arg(d.id, -12).arg(d.name);
        }
    }

    QString finalId = bestId.isEmpty() ? getPlatformFallbackDevice() : bestId;

    qDebug() << "\n>>> SELECTED PLAYBACK DEVICE:" << finalId;
    if (!bestId.isEmpty()) {
        qDebug() << "    Reason: Highest priority →" << bestName << "(score:" << bestScore << ")";
    } else {
        qDebug() << "    Reason: No playback device found → using platform default";
    }
    qDebug() << "==================================================\n";
}

QStringList AudioHelper::getAllDevices() const
{
    QStringList list;
    auto devices = scanAllDevicesOnce();
    for (const auto &d : devices) {
        list << QString("%1|%2|%3").arg(d.id, d.type, d.name);
    }
    return list;
}

QString AudioHelper::getPriorityDevice() const
{
    auto devices = scanAllDevicesOnce();
    QString bestId;
    QString bestName;
    int bestScore = -1;

    for (const auto &d : devices) {
        if (d.type == "Playback" && d.score > bestScore) {
            bestScore = d.score;
            bestId = d.id;
            bestName = d.name;
        }
    }

    if (bestId.isEmpty()) {
        QString fallback = getPlatformFallbackDevice();
        return fallback + "|Platform Default";
    }

    return bestId + "|" + bestName;
}

QString AudioHelper::getPriorityDeviceId() const
{
    QString full = getPriorityDevice();
    if (full.contains('|')) {
        return full.split('|').first().trimmed();
    }
    return "default";
}