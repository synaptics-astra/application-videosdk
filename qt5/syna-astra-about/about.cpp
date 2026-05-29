// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "about.h"

AboutManager::AboutManager(QObject *parent) : QObject(parent) {}

QString AboutManager::fetchAstraVersion() {
    QProcess process;
    process.start("cat /etc/astra_version");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Version";

    QString version = process.readAllStandardOutput().trimmed();
    return version.isEmpty() ? "Unknown Version" : version;
}

QString AboutManager::fetchHwVersion() {
    QProcess process;
    process.start("cat /etc/hwrevision");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Version";

    QString version = process.readAllStandardOutput().trimmed();
    return version.isEmpty() ? "Unknown Version" : version;
}

QString AboutManager::fetchLinuxVersion() {
    QProcess process;
    process.start("uname -a");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Version";

    QString version = process.readAllStandardOutput().trimmed();
    return version.isEmpty() ? "Unknown Version" : version;
}

QString AboutManager::getMachine() {
    QProcess process;
    process.start("uname -n");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Machine";

    QString machine = process.readAllStandardOutput().trimmed();
    return machine.isEmpty() ? "Unknown Machine" : machine;
}

QString AboutManager::fetchIpAddress()
{
    QProcess process;
    process.start("sh", QStringList() << "-c"
        << "ifconfig 2>/dev/null | grep -E 'inet[^6]' | grep -v 127.0.0.1 | awk '{print $2}' | cut -d: -f2 | head -n1");

    if (!process.waitForFinished(3000) || process.exitCode() != 0)
        return "Unknown IP";

    QString ip = process.readAllStandardOutput().trimmed();
    return ip.isEmpty() ? "Unknown IP" : ip;
}