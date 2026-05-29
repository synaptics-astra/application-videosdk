// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <glib.h>
#include <gst/rtsp/gstrtsp.h>
#include <gst/sdp/gstsdpmessage.h>
#include <QDir>
#include <QFile>
#include <QDebug>
#include "settingsmanager.h"

const QString SettingsManager::CONFIG_FILE_PATH = "/home/root/demos/configs/synaexplorer_config.txt";

void SettingsManager::createConfig(const QString &filePath) {
    QDir dir = QFileInfo(filePath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile configFile(filePath);
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);
        out << "\n[HOST_IP]\n";
        configFile.close();
    } else {
        qWarning() << "Failed to create config file at" << CONFIG_FILE_PATH
                   << "Error:" << configFile.errorString();
    }
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
{
    loadConfig(SettingsManager::CONFIG_FILE_PATH);
}

void SettingsManager::saveConfig(const QString &filePath) {
   QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "[HOST_IP]\n";
        out << hostIP << "\n";
        file.close();
    }
}

void SettingsManager::loadConfig(const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString currentSection;
        hostIP.clear();

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            if (line == "[HOST_IP]") {
                currentSection = "HOST_IP";
            } else if (currentSection == "HOST_IP") {
                hostIP = line;
            }
        }

        file.close();
        emit hostIPChanged();
    } else {
        createConfig(SettingsManager::CONFIG_FILE_PATH);
    }
}

void SettingsManager::setHostIP(const QString &addr) {
    hostIP = addr;
    emit hostIPChanged();
}

QString SettingsManager::getHostIP() const {
    return hostIP;
}

bool SettingsManager::setData(int id, const QVariantMap &data) {

    SynaSettingsFramework settingsFramework;

    switch (id) {
        case SETTINGS_ID_USB_FORMAT_PRIORITIES: {
            UsbFormatPrioritiesData usbData;
            const char* defaultFormatPriorities[] = {"YUY2", "NV12", "H264", "MJPEG"};
            if (data.contains("formatPriorities")) {
                QVariantList priorities = data["formatPriorities"].toList();
                if (priorities.size() != 4) {
                    for (size_t i = 0; i < 4; i++) {
                        strncpy(usbData.formatPriorities[i], defaultFormatPriorities[i], sizeof(usbData.formatPriorities[i]) - 1);
                        usbData.formatPriorities[i][sizeof(usbData.formatPriorities[i]) - 1] = '\0';
                    }
                } else {
                    for (int i = 0; i < 4; i++) {
                        strncpy(usbData.formatPriorities[i], priorities[i].toString().toUtf8().constData(), sizeof(usbData.formatPriorities[i]) - 1);
                        usbData.formatPriorities[i][sizeof(usbData.formatPriorities[i]) - 1] = '\0';
                    }
                }
            } else {
                for (size_t i = 0; i < 4; i++) {
                    strncpy(usbData.formatPriorities[i], defaultFormatPriorities[i], sizeof(usbData.formatPriorities[i]) - 1);
                    usbData.formatPriorities[i][sizeof(usbData.formatPriorities[i]) - 1] = '\0';
                }
            }

            if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsWrite((SETTINGS_ID_e)id, &usbData, sizeof(UsbFormatPrioritiesData))) {
                qDebug() << "Failed to write USB FORMAT Priorities";
                return false;
            }
            emit prioritiesChanged();
            break;
        }

        case SETTINGS_ID_RTSP_URLS: {
            RtspSettingsData rtspData;
            const char* defaultRtspUrls[] = {"", "", "", ""};
            if (data.contains("rtspUrls")) {
                QVariantList urls = data["rtspUrls"].toList();
                for (size_t i = 0; i < std::min<size_t>(4, urls.size()); i++) {
                    if (!urls[i].toString().isEmpty()) {
                        strncpy(rtspData.rtspUrls[i], urls[i].toString().toUtf8().constData(), sizeof(rtspData.rtspUrls[i]) - 1);
                        rtspData.rtspUrls[i][sizeof(rtspData.rtspUrls[i]) - 1] = '\0';
                    } else {
                        rtspData.rtspUrls[i][0] = '\0';
                    }
                }
                for (size_t i = urls.size(); i < 4; i++) {
                    strncpy(rtspData.rtspUrls[i], defaultRtspUrls[i], sizeof(rtspData.rtspUrls[i]) - 1);
                    rtspData.rtspUrls[i][sizeof(rtspData.rtspUrls[i]) - 1] = '\0';
                }
            } else {
                qDebug() << "No rtspUrls key in data";
                for (size_t i = 0; i < 4; i++) {
                    strncpy(rtspData.rtspUrls[i], defaultRtspUrls[i], sizeof(rtspData.rtspUrls[i]) - 1);
                    rtspData.rtspUrls[i][sizeof(rtspData.rtspUrls[i]) - 1] = '\0';
                }
            }

            SETTINGS_ERROR_e error = settingsFramework.synaSettingsWrite((SETTINGS_ID_e)id, &rtspData, sizeof(RtspSettingsData));
            if (error != SETTINGS_ERROR_NONE) {
                qDebug() << "Failed to write RTSP Settings, Error:" << error << "Data Size:" << sizeof(RtspSettingsData);
                return false;
            }
            emit rtspUrlsChanged();
            break;
        }

        default: {
            qDebug() << "Invalid ID";
            return false;
        }
    }

    return true;
}

QVariantMap SettingsManager::getData(int id) const {
    QVariantMap result;
    SynaSettingsFramework settingsFramework;

    switch (id) {
        case SETTINGS_ID_USB_FORMAT_PRIORITIES: {
            UsbFormatPrioritiesData usbData;
            QVariantList priorities;
            int size = sizeof(UsbFormatPrioritiesData);
            if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsRead((SETTINGS_ID_e)id, &usbData, &size)) {
                priorities = {"YUY2", "NV12", "H264", "MJPEG"};
            } else {
                for (int i = 0; i < 4; i++) {
                    priorities.append(QString::fromUtf8(usbData.formatPriorities[i]));
                }
            }
            result["formatPriorities"] = priorities;
            return result;
        }

        case SETTINGS_ID_RTSP_URLS: {
            RtspSettingsData rtspData;
            QVariantList urls;
            int size = sizeof(RtspSettingsData);
            if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsRead((SETTINGS_ID_e)id, &rtspData, &size)) {
                qDebug() << "Failed to read RTSP Settings";
                urls = {"", "", "", ""};
            } else {
                for (int i = 0; i < 4; i++) {
                    urls.append(QString::fromUtf8(rtspData.rtspUrls[i]));
                }
            }
            result["rtspUrls"] = urls;
            return result;
        }

        default: {
            qDebug() << "Invalid ID";
            return result;
        }
    }
    return result;
}

QStringList SettingsManager::rtspUrls() const {
    QVariantMap rtspData = getData(SETTINGS_ID_RTSP_URLS);
    return rtspData["rtspUrls"].toStringList();
}

QStringList SettingsManager::formatPriorities() const {
    QVariantMap formatData = getData(SETTINGS_ID_USB_FORMAT_PRIORITIES);
    return formatData["formatPriorities"].toStringList();
}

bool SettingsManager::checkData(int id) {
    SynaSettingsFramework settingsFramework;
    bool dataExists = settingsFramework.checkFile((SETTINGS_ID_e)id) == SETTINGS_ERROR_NONE;

    if (!dataExists) {
        switch (id) {
            case SETTINGS_ID_USB_FORMAT_PRIORITIES: {
                QVariantMap formatMap;
                formatMap.insert("formatPriorities", QStringList({"YUY2", "NV12", "H264", "MJPEG"}));
                if (!setData(id, formatMap)) {
                    qWarning() << "Failed to set default USB format priorities";
                    return false;
                }
                return true;
            }

            case SETTINGS_ID_RTSP_URLS: {
                QVariantMap rtspMap;
                rtspMap.insert("rtspUrls", QStringList({"", "", "", ""}));
                if (!setData(id, rtspMap)) {
                    qWarning() << "Failed to set default RTSP URLs";
                    return false;
                }
                return true;
            }

            default: {
                qWarning() << "Invalid settings ID: " << id;
                return false;
            }
        }
    }
    return true;
}