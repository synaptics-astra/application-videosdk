// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "syna-settings-framework.h"
#include <sys/stat.h>

#define SETTINGS_DIR "/home/root/settings"

SETTINGS_ERROR_e SynaSettingsFramework::synaSettingsWrite(SETTINGS_ID_e id, void *data, int size) {

    if (SETTINGS_ID_MAX <= id)
        return SETTINGS_ERROR_INVALID_ID;

    char filename[1024];
    snprintf(filename, 1024, "%s/%d.data", SETTINGS_DIR, id);

    /* Check for settings directory */
    struct stat st = {0};
    if (stat(SETTINGS_DIR, &st) == -1) {
        if (mkdir(SETTINGS_DIR, 0777) != 0) {
            std::cout << "Failed to create directory: " << SETTINGS_DIR;
            return SETTINGS_ERROR_IO;
        }
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        std::cout << "Failed to open/create file: " << filename;
        return SETTINGS_ERROR_IO;
    }

    if (1 != fwrite(data, size, 1, file)) {
        std::cout << "Failed to write to " << filename;
        fclose(file);
        return SETTINGS_ERROR_IO;
    }

    fclose(file);
    return SETTINGS_ERROR_NONE;
}

SETTINGS_ERROR_e SynaSettingsFramework::synaSettingsRead(SETTINGS_ID_e id, void *data,
    int *size) {

    int read_size = 0;

    if (SETTINGS_ID_MAX <= id)
        return SETTINGS_ERROR_INVALID_ID;

    char filename[1024];
    snprintf(filename, 1024, "%s/%d.data", SETTINGS_DIR, id);

    FILE *file = fopen(filename, "rb");
    if (!file) {
        std::cout << "Failed to open file: " << filename;
        return SETTINGS_ERROR_IO;
    }

    read_size = fread(data, *size, 1, file);
    fclose(file);
    if (read_size != 1) {
        *size = read_size;
        std::cout << "Size mismatch: %s" << filename;
        return SETTINGS_ERROR_SIZE_MISMATCH;
    }

    return SETTINGS_ERROR_NONE;
}

SETTINGS_ERROR_e SynaSettingsFramework::synaSettingsDelete(SETTINGS_ID_e id) {

    if (SETTINGS_ID_MAX <= id)
        return SETTINGS_ERROR_INVALID_ID;

    char filename[1024];
    snprintf(filename, 1024, "%s/%d.data", SETTINGS_DIR, id);

    remove(filename);

    return SETTINGS_ERROR_NONE;
}

