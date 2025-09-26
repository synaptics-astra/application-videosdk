// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "processhelper.h"
#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QTextStream>

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

QString ProcessHelper::readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to read file:" << path;
        return "";
    }
    QTextStream in(&file);
    QString contents = in.readAll();
    file.close();
    return contents;
}

bool ProcessHelper::writeFile(const QString &path, const QString &contents)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to write file:" << path;
        return false;
    }
    QTextStream out(&file);
    out << contents;
    file.close();
    return true;
}

QString ProcessHelper::findFile(const QStringList &paths)
{
    for (const QString &path : paths) {
        if (fileExists(path)) {
            return path;
        }
    }
    return "";
}