// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef BLUETOOTHAPP_H
#define BLUETOOTHAPP_H

#include <QObject>
#include <QProcess>
#include <QSettings>

class Bluetoothapp : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool bluetoothState READ bluetoothState NOTIFY bluetoothStateChanged)

public:
    explicit Bluetoothapp(QObject *parent = nullptr);
    bool bluetoothState() const;
    Q_INVOKABLE void togglePythonScript(bool state);
    Q_INVOKABLE void showAvailableDevices();
    Q_INVOKABLE void manageKnownDevices();

signals:
    void bluetoothStateChanged();

private:
    void setBluetoothState(bool state);
    QProcess *process;
    QSettings settings;
};

#endif
