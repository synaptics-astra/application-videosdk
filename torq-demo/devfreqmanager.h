// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef DEVFREQMANAGER_H
#define DEVFREQMANAGER_H

#include <QString>

class DevFreqManager {
public:
    static bool initializeGovernor();
    static bool boostNPU();
    static bool throttleNPU();

private:
    static const QString DEVFREQ_GOVERNOR_PATH;
    static const QString DEVFREQ_SETFREQ_PATH;
    static const QString DEVFREQ_MAX_FREQ_PATH;
    static const QString DEVFREQ_MIN_FREQ_PATH;

    static QString readSysfsValue(const QString &path);
    static bool writeSysfsValue(const QString &path, const QString &value);
};

#endif // DEVFREQMANAGER_H
