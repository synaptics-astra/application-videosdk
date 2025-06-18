// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SYNA_SETTINGS_FRAMEWORK_H
#define SYNA_SETTINGS_FRAMEWORK_H

#include <iostream>
#include <fstream>
#include <string>

//#define SAMPLE_MODULE

typedef enum {
    SETTINGS_ID_DISPLAY_SETTINGS,
#ifdef SAMPLE_MODULE
    SETTINGS_ID_SAMPLE_MODULE,
#endif
    SETTINGS_ID_MAX
}SETTINGS_ID_e;

typedef enum {
    SETTINGS_ERROR_NONE,
    SETTINGS_ERROR_INVALID_ID,
    SETTINGS_ERROR_IO,
    SETTINGS_ERROR_SIZE_MISMATCH,
    SETTINGS_ERROR_UNKNOWN,
    SETTINGS_ERROR_MAX,
}SETTINGS_ERROR_e;

class SynaSettingsFramework {
public:
    SETTINGS_ERROR_e synaSettingsWrite(SETTINGS_ID_e id, void *data, int size);
    SETTINGS_ERROR_e synaSettingsRead(SETTINGS_ID_e id, void *data, int *size);
    SETTINGS_ERROR_e synaSettingsDelete(SETTINGS_ID_e id);
};

#endif // SYNA_SETTINGS_FRAMEWORK_H
