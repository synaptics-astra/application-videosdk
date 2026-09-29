// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "devfreqmanager.h"
#include <QFile>
#include <QDebug>

const QString DevFreqManager::DEVFREQ_GOVERNOR_PATH = "/sys/class/devfreq/f7600000.synpu/governor";
const QString DevFreqManager::DEVFREQ_SETFREQ_PATH = "/sys/class/devfreq/f7600000.synpu/userspace/set_freq";
const QString DevFreqManager::DEVFREQ_MAX_FREQ_PATH = "/sys/class/devfreq/f7600000.synpu/max_freq";
const QString DevFreqManager::DEVFREQ_MIN_FREQ_PATH = "/sys/class/devfreq/f7600000.synpu/min_freq";

QString DevFreqManager::readSysfsValue(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "DevFreqManager: Failed to open" << path;
        return QString();
    }

    QString value = QString(file.readAll()).trimmed();
    file.close();

    return value;
}

bool DevFreqManager::writeSysfsValue(const QString &path, const QString &value)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "DevFreqManager: Failed to open" << path;
        return false;
    }

    qint64 bytesWritten = file.write(value.toLatin1());
    file.close();

    if (bytesWritten == -1) {
        qWarning() << "DevFreqManager: Failed to write to" << path;
        return false;
    }

    return true;
}

bool DevFreqManager::initializeGovernor()
{
    qDebug() << "DevFreqManager: Initializing devfreq governor to userspace mode";
    return writeSysfsValue(DEVFREQ_GOVERNOR_PATH, "userspace\n");
}

bool DevFreqManager::boostNPU()
{
    qDebug() << "DevFreqManager: Boosting NPU to maximum frequency";
    QString maxFreq = readSysfsValue(DEVFREQ_MAX_FREQ_PATH);
    if (maxFreq.isEmpty()) {
        qWarning() << "DevFreqManager: Failed to read max frequency";
        return false;
    }
    return writeSysfsValue(DEVFREQ_SETFREQ_PATH, maxFreq + "\n");
}

bool DevFreqManager::throttleNPU()
{
    qDebug() << "DevFreqManager: Throttling NPU to minimum frequency";
    QString minFreq = readSysfsValue(DEVFREQ_MIN_FREQ_PATH);
    if (minFreq.isEmpty()) {
        qWarning() << "DevFreqManager: Failed to read min frequency";
        return false;
    }
    return writeSysfsValue(DEVFREQ_SETFREQ_PATH, minFreq + "\n");
}
