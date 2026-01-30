// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <QObject>
#include <QStringList>

class AudioHelper : public QObject
{
    Q_OBJECT
public:
    explicit AudioHelper(QObject *parent = nullptr);

    Q_INVOKABLE void printDevices() const;
    Q_INVOKABLE QStringList getAllDevices() const;
    Q_INVOKABLE QString getPriorityDevice() const;
    Q_INVOKABLE QString getPriorityDeviceId() const;

private:
    struct DeviceInfo {
        QString id;
        QString name;
        QString type;
        int score = 0;
    };

    void forceAlsaRescan() const;
    QVector<DeviceInfo> scanAllDevicesOnce() const;

    QString detectMachine() const;
    QString getPlatformFallbackDevice() const;
    int scorePlaybackDevice(const QString &name, const QString &id) const;
};

#endif // AUDIOHELPER_H