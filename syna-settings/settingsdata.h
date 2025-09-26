// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SETTINGS_DATA_H
#define SETTINGS_DATA_H

#include <iostream>
#include <string>
#include "syna-settings-framework.h"

#ifdef SAMPLE_MODULE
struct SampleModuleData {
    char textBox1_str[128];
    char textBox2_str[128];
    char textBoxA_str[128];
    char textBoxB_str[128];
    int dropDownMenu_int;
    char dropDownMenu_prio0_str[128];
    char dropDownMenu_prio1_str[128];
    char dropDownMenu_prio2_str[128];
    char dropDownMenu_prio3_str[128];
    char dropDownMenu_prio4_str[128];
};
#endif
typedef struct {
    char selectedPanel[256];
} DisplaySettingsData;

typedef struct {
    char formatPriorities[4][16];
} UsbFormatPrioritiesData;

typedef struct {
    char rtspUrls[4][256];
} RtspSettingsData;

#endif // SETTINGS_DATA_H
