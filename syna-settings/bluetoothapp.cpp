// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "bluetoothapp.h"
#include <QDebug>
#include <QtGlobal>
#include <QFile>
#include <csignal>
#include <cerrno>
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
    if (process->state() == QProcess::NotRunning) {
        process->start("python3", QStringList() << "/usr/bin/bl_app.py");
        qDebug() << "Python script started";
        return;
    }
    if (scanInProgress()) {
        /* A scan is currently running. Do not interrupt it; */
        qDebug() << "Scan in progress, Do nothing";
        return;
    }

    process->terminate();
    if (!process->waitForFinished(1000)) {
        qCritical() << "Graceful terminate timed out, forcing kill";
        process->kill();
        process->waitForFinished(500);
    }
    process->start("python3", QStringList() << "/usr/bin/bl_app.py");
    qDebug() << "Python script started";
}

bool Bluetoothapp::scanInProgress() const {
    const QString flagPath = "/tmp/bt_scan_in_progress";
    QFile flag(flagPath);
    if (!flag.exists())
        return false;

    if (!flag.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot read scan flag, treating as no scan in progress";
        flag.remove();
        return false;
    }
    bool ok = false;
    qint64 pid = QString::fromLatin1(flag.readAll()).trimmed().toLongLong(&ok);
    flag.close();

    if (!ok || pid <= 0) {
        qCritical() << "Scan flag has invalid PID, removing stale flag";
        flag.remove();
        return false;
    }
    // kill(pid, 0) does not send a signal; it only checks whether the PID exists.
    if (::kill(static_cast<pid_t>(pid), 0) != 0 && errno == ESRCH) {
        qInfo() << "Scan flag PID" << pid << "is not alive, removing stale flag";
        flag.remove();
        return false;
    }
    return true;
}

void Bluetoothapp::manageKnownDevices() {
    bool success = QProcess::startDetached("python3", QStringList() << "/usr/bin/known_devices.py");
    if (success) {
        qDebug() << "known_devices.py started successfully";
    } else {
        qCritical() << "Failed to start known_devices.py";
    }
}
