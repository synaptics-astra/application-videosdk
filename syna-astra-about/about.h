// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QDebug>

class AboutManager : public QObject {
    Q_OBJECT

public:
    explicit AboutManager(QObject *parent = nullptr);
    Q_INVOKABLE QString fetchAstraVersion();
    Q_INVOKABLE QString fetchHwVersion();
    Q_INVOKABLE QString fetchLinuxVersion();
    Q_INVOKABLE QString getMachine();
};

#endif // ABOUT_H