// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "bluetoothapp.h"
#include <QDebug>
#include <QtGlobal>
Bluetoothapp::Bluetoothapp(QObject *parent) : QObject(parent), process(new QProcess(this)), settings("STATE", "BLUETOOTH") {
    bool lastState = settings.value("bluetoothState", false).toBool();
    setBluetoothState(lastState);
}

void Bluetoothapp::togglePythonScript(bool state) {
    setBluetoothState(state);
    if (state) {
        int exitCode = QProcess::execute("bluetoothctl", {"power", "on"});
        if (exitCode == 0) {
            qDebug() << "Bluetooth powered on successfully.";
        } else {
            qCritical() << "Failed to power on Bluetooth. Return code:" << exitCode;
        }
        emit bluetoothStateChanged();
    } else {
        int exitCode = QProcess::execute("bluetoothctl", {"power", "off"});
        if (exitCode == 0) {
            qDebug() << "Bluetooth powered off successfully.";
        } else {
            qCritical() << "Failed to power off Bluetooth. Return code:" << exitCode;
        }
    }
}

bool Bluetoothapp::bluetoothState() const {
    return settings.value("bluetoothState", false).toBool();
}

void Bluetoothapp::setBluetoothState(bool state) {
    if (bluetoothState() != state) {
        settings.setValue("bluetoothState", state);
        emit bluetoothStateChanged();
    }
}

void Bluetoothapp::showAvailableDevices() {
    if (process->state() != QProcess::Running) {
        process->start("python3", QStringList() << "/usr/bin/bl_app.py");
        qDebug() << "Python script started";
    } else {
        qCritical() << "Python script is already running";
    }
}

void Bluetoothapp::manageKnownDevices() {
    bool success = QProcess::startDetached("python3", QStringList() << "/usr/bin/known_devices.py");
    if (success) {
        qDebug() << "known_devices.py started successfully";
    } else {
        qCritical() << "Failed to start known_devices.py";
    }
}
