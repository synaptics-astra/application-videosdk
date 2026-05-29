// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "wifiapp.h"
#include <QDebug>
#include <QtGlobal>
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
    if (process->state() != QProcess::Running) {
        process->start("python3", QStringList() << "/usr/bin/app.py");
        qDebug() << "Python script started";
    } else {
        qCritical() << "Python script is already running";
    }
}

void Wifiapp::manageKnownNetworks() {
    bool success = QProcess::startDetached("python3", QStringList() << "/usr/bin/known_networks.py");
    if (success) {
        qDebug() << "known_networks.py started successfully";
    } else {
        qCritical() << "Failed to start known_networks.py";
    }
}
