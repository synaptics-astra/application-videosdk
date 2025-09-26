// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

#include <QObject>
#include <QString>
#include <QStringList>

class ProcessHelper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessHelper(QObject *parent = nullptr);

    Q_INVOKABLE bool fileExists(const QString &path);
    Q_INVOKABLE void executeCommand(const QString &command);
    Q_INVOKABLE QString readFile(const QString &path);
    Q_INVOKABLE bool writeFile(const QString &path, const QString &contents);
    Q_INVOKABLE QString findFile(const QStringList &paths);

signals:
    void statusUpdated(const QString &message);

};

#endif // PROCESSHELPER_H
