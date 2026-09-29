// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "wifiapp.h"
#include <QDebug>
#include <QtGlobal>
#include <QFile>
#include <csignal>
#include <cerrno>
Wifiapp::Wifiapp(QObject *parent) : QObject(parent), process(new QProcess(this)), settings("STATE", "WIFI") {
    bool lastState = settings.value("wifiState", false).toBool();
    setWifiState(lastState, false);
}

void Wifiapp::togglePythonScript(bool state) {
    setWifiState(state);
    if (state) {
        int exitCode = QProcess::execute("systemctl restart wpa_supplicant@wlan0.service");
        if (exitCode == 0) {
            qDebug() << "wpa_supplicant@wlan0.service started successfully";
        } else {
            qCritical() << "Failed to start wpa_supplicant@wlan0.service";
        }
        emit wifiStateChanged();
    } else {
        int exitCode = QProcess::execute("ifconfig", QStringList() << "wlan0" << "down");
        if (exitCode == 0) {
            qDebug() << "wpa_supplicant@wlan0.service stopped successfully";
        } else {
            qCritical() << "Failed to stop wpa_supplicant@wlan0.service";
        }
    }
}

bool Wifiapp::wifiState() const {
    return settings.value("wifiState", false).toBool();
}

void Wifiapp::setWifiState(bool state, bool emitSignal) {
    if (wifiState() != state) {
        settings.setValue("wifiState", state);
        if (emitSignal)
            emit wifiStateChanged();
    }
}

void Wifiapp::showAvailableNetworks() {
    if (process->state() == QProcess::NotRunning) {
        process->start("python3", QStringList() << "/usr/bin/app.py");
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
    process->start("python3", QStringList() << "/usr/bin/app.py");
    qDebug() << "Python script started";
}

bool Wifiapp::scanInProgress() const {
    const QString flagPath = "/tmp/wifi_scan_in_progress";
    QFile flag(flagPath);
    if (!flag.exists())
        return false;

    /* The flag contains the PID of the scanning process. If that process is no
       longer alive, the flag is stale (e.g. the Python app crashed mid-scan);
       remove it so it does not permanently block future scans. */
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
    /* kill(pid, 0) does not send a signal; it only checks whether the PID exists. */
    if (::kill(static_cast<pid_t>(pid), 0) != 0 && errno == ESRCH) {
        qInfo() << "Scan flag PID" << pid << "is not alive, removing stale flag";
        flag.remove();
        return false;
    }
    return true;
}

void Wifiapp::manageKnownNetworks() {
    bool success = QProcess::startDetached("python3", QStringList() << "/usr/bin/known_networks.py");
    if (success) {
        qDebug() << "known_networks.py started successfully";
    } else {
        qCritical() << "Failed to start known_networks.py";
    }
}
