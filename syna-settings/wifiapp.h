// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef WIFIAPP_H
#define WIFIAPP_H

#include <QObject>
#include <QProcess>
#include <QSettings>

class Wifiapp : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool wifiState READ wifiState NOTIFY wifiStateChanged)

public:
    explicit Wifiapp(QObject *parent = nullptr);
    bool wifiState() const;
    Q_INVOKABLE void togglePythonScript(bool state);
    Q_INVOKABLE void showAvailableNetworks();
    Q_INVOKABLE void manageKnownNetworks();

signals:
    void wifiStateChanged();

private:
    void setWifiState(bool state, bool emitSignal = true);
    bool scanInProgress() const;
    QProcess *process;
    QSettings settings;
};

#endif
