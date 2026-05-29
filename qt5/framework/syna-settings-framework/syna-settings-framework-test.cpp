// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "syna-settings-framework.h"
#include <stdio.h>
#include <cstring>

/*
The structure should be prepared and maintained by individual application as per need
Try to add a fixed sized char array than std::string to avoid size related issue as same
structure size is used to read from file
*/
typedef struct {
    int param1;
    char param2[256];
    char param3[256];
    int param4;
} TEST_SETTINGS_t;


int main() {
    TEST_SETTINGS_t data, readData;

#ifdef SAMPLE_MODULE
    data.param1 = 1;
    std::strncpy(data.param2, "Settings test first run", (sizeof(data.param2) - 1));
    std::strncpy(data.param3, "Another parameter", (sizeof(data.param3) - 1));
    data.param4 = 100;
    std::cout << "Settings to be stored: " << data.param1 << ", " << data.param2 << ", " << data.param3 << ", " << data.param4 << std::endl;

    SynaSettingsFramework settingsFramework;

    if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsWrite(SETTINGS_ID_SAMPLE_MODULE, &data, sizeof(TEST_SETTINGS_t))) {
        std::cout << "Failed to write settings\n";
        return -1;
    }
    std::cout << "Settings wrote\n" << sizeof(TEST_SETTINGS_t);

    int size = sizeof(TEST_SETTINGS_t);
    if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsRead(SETTINGS_ID_SAMPLE_MODULE, &readData, &size)) {
        std::cout << "Failed to read settings\n";
        return -1;
    }
    std::cout << "Settings read: " << readData.param1 << ", " << readData.param2 << ", " << readData.param3 << ", " << readData.param4 << std::endl;

    data.param1 = 2;
    std::string str = "Another parameter modified";
    std::strncpy(data.param2, "Settings parameter second run", (sizeof(data.param2) - 1));
    std::strncpy(data.param3, str.c_str(), (sizeof(data.param3) - 1));
    data.param4 = 105;

    if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsWrite(SETTINGS_ID_SAMPLE_MODULE, &data, sizeof(TEST_SETTINGS_t))) {
        std::cout << "Failed to write settings\n";
        return -1;
    }
    std::cout << "Settings updated\n";

    if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsRead(SETTINGS_ID_SAMPLE_MODULE, &readData, &size)) {
        std::cout << "Failed to read settings\n";
        return -1;
    }
    std::cout << "Settings read: " << readData.param1 << ", " << readData.param2 << ", " << readData.param3 << ", " << readData.param4 << std::endl;

    settingsFramework.synaSettingsDelete(SETTINGS_ID_SAMPLE_MODULE);
    std::cout << "Settings deleted\n";
#else
    std::cout << "SAMPLE_MODULE is not defined. Enable in syna-settings-framework at compile time.\n";
#endif

    return 0;
}
