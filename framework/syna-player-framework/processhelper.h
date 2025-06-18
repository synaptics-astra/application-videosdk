// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

#include <QObject>
#include <QString>

class ProcessHelper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessHelper(QObject *parent = nullptr);

    Q_INVOKABLE bool fileExists(const QString &path);
    Q_INVOKABLE void executeCommand(const QString &command);

signals:
    void statusUpdated(const QString &message);

};

#endif // PROCESSHELPER_H

