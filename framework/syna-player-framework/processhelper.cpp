// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "processhelper.h"
#include <QFile>
#include <QProcess>
#include <QDebug>

ProcessHelper::ProcessHelper(QObject *parent) : QObject(parent)
{
}

bool ProcessHelper::fileExists(const QString &path)
{
    QFile file(path);
    return file.exists();
}

void ProcessHelper::executeCommand(const QString &command)
{
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << command);
    if (!process.waitForFinished(-1)) {
        qDebug() << "Command failed or timed out:" << command;
        emit statusUpdated("Command execution failed or timed out");
        return;
    }

    QString stdoutOutput = process.readAllStandardOutput().trimmed();
    QString stderrOutput = process.readAllStandardError().trimmed();

    if (!stdoutOutput.isEmpty()) {
        emit statusUpdated(stdoutOutput);
    } else if (!stderrOutput.isEmpty()) {
        qDebug() << "Command error:" << command << "Stderr:" << stderrOutput;
        emit statusUpdated("Error: " + stderrOutput);
    } else {
        emit statusUpdated("Command executed successfully");
    }
}

