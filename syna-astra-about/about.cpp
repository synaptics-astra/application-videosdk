// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "about.h"

#include <QRegularExpression>
#include <QProcess>
#include <QString>

AboutManager::AboutManager(QObject *parent) : QObject(parent) {}

QString AboutManager::fetchAstraVersion() {
    QProcess process;
    process.start("cat",QStringList{"/etc/astra_version"});
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Version";

    QString version = process.readAllStandardOutput().trimmed();
    return version.isEmpty() ? "Unknown Version" : version;
}

QString AboutManager::fetchHwVersion() {
    QProcess process;
    process.start("cat",QStringList{"/etc/hwrevision"});
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Version";

    QString version = process.readAllStandardOutput().trimmed();
    return version.isEmpty() ? "Unknown Version" : version;
}

QString AboutManager::fetchLinuxVersion() {
    QProcess process;
    process.start("uname",QStringList() << "-a");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Version";

    QString version = process.readAllStandardOutput().trimmed();
    return version.isEmpty() ? "Unknown Version" : version;
}

QString AboutManager::getMachine() {
    QProcess process;
    process.start("uname", QStringList() << "-n");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Machine";

    QString machine = process.readAllStandardOutput().trimmed();
    if (machine == "sl2619-coralboard") {
        return "sl2619";
    }
    return machine.isEmpty() ? "Unknown Machine" : machine;
}

static QString getRoutedIPv4()
{
    QProcess process;

    process.start("ip",
                  QStringList() << "route"
                                << "get"
                                << "8.8.8.8");

    if (!process.waitForFinished(1000))
        return {};

    const QString output =
        QString::fromUtf8(process.readAllStandardOutput());

    const QRegularExpression re(
        R"(src\s+((?:\d{1,3}\.){3}\d{1,3}))");

    const QRegularExpressionMatch match =
        re.match(output);

    if (match.hasMatch())
        return match.captured(1);

    return {};
}

QString AboutManager::fetchIpAddress()
{
    const QString ip = getRoutedIPv4();

    return ip.isEmpty()
           ? QStringLiteral("Unknown IP")
           : ip;
}