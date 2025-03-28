// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SETTINGS_CONTROL_H
#define SETTINGS_CONTROL_H

#include <QObject>
#include <QVariantMap>
#include <QDebug>

#include <cstdio>
#include <iostream>
#include <cstring>

#include <syna-settings-framework.h>
#include <settingsdata.h>


class SettingsControl : public QObject {
    Q_OBJECT

public:
    explicit SettingsControl(QObject *parent = nullptr) {}

    Q_INVOKABLE bool setData(int id, const QVariantMap &data) {

        SynaSettingsFramework settingsFramework;

        switch (id) {
#ifdef SAMPLE_MODULE
            case SETTINGS_ID_SAMPLE_MODULE: {
                SampleModuleData sampleModuleData;

                std::strncpy(sampleModuleData.textBox1_str,
                    data["textBox1_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.textBox1_str) - 1));
                std::strncpy(sampleModuleData.textBox2_str,
                    data["textBox2_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.textBox2_str) - 1));
                std::strncpy(sampleModuleData.textBoxA_str,
                    data["textBoxA_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.textBoxA_str) - 1));
                std::strncpy(sampleModuleData.textBoxB_str,
                    data["textBoxB_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.textBoxB_str) - 1));
                sampleModuleData.dropDownMenu_int = data["dropDownMenu_int"].toInt();
                std::strncpy(sampleModuleData.dropDownMenu_prio0_str,
                    data["dropDownMenu_prio0_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.dropDownMenu_prio0_str) - 1));
                std::strncpy(sampleModuleData.dropDownMenu_prio1_str,
                    data["dropDownMenu_prio1_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.dropDownMenu_prio1_str) - 1));
                std::strncpy(sampleModuleData.dropDownMenu_prio2_str,
                    data["dropDownMenu_prio2_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.dropDownMenu_prio2_str) - 1));
                std::strncpy(sampleModuleData.dropDownMenu_prio3_str,
                    data["dropDownMenu_prio3_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.dropDownMenu_prio3_str) - 1));
                std::strncpy(sampleModuleData.dropDownMenu_prio4_str,
                    data["dropDownMenu_prio4_str"].toString().toStdString().c_str(),
                    (sizeof(sampleModuleData.dropDownMenu_prio4_str) - 1));

                if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsWrite((SETTINGS_ID_e)id, &sampleModuleData, sizeof(SampleModuleData))) {
                    qDebug() << "Failed to write settings";
                    return false;
                }

                break;
            }
#endif
            case SETTINGS_ID_DISPLAY_SETTINGS: { // trying to add new case here for display settings
                DisplaySettingsData displayData;
                if (data.contains("selectedPanel")) { // to ensure key exist before using
                    strncpy(displayData.selectedPanel, data["selectedPanel"].toString().toStdString().c_str(), sizeof(displayData.selectedPanel) - 1);
                     displayData.selectedPanel[sizeof(displayData.selectedPanel) -1] = '\0'; // ensure null terminate
                } else {
                     qDebug() << "Inside getdata-> SelectedPanel value not found in data !'";
                     return false;
                }

                if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsWrite((SETTINGS_ID_e)id , &displayData, sizeof(DisplaySettingsData))) {
                    qDebug() << "Inside getdata->failed to write Display Settings";
                    return false;
                }

                break;
            }

            default: {
                qDebug() << "Invalid ID";
                return false;
            }
        }

        return true;
    }

    // Function to read struct from file and return as QVariantMap
    Q_INVOKABLE QVariantMap getData(int id) {
        QVariantMap result;
        SynaSettingsFramework settingsFramework;

        switch (id) {
#ifdef SAMPLE_MODULE
            case SETTINGS_ID_SAMPLE_MODULE: {
                SampleModuleData data;
                int size = sizeof(SampleModuleData);
                if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsRead((SETTINGS_ID_e)SETTINGS_ID_SAMPLE_MODULE, &data, &size)) {
                    qDebug() << "Failed to read settings";
                    return result;
                }

                result["textBox1_str"] = QString::fromUtf8(data.textBox1_str);
                result["textBox2_str"] = QString::fromUtf8(data.textBox2_str);
                result["textBoxA_str"] = QString::fromUtf8(data.textBoxA_str);
                result["textBoxB_str"] = QString::fromUtf8(data.textBoxB_str);
                result["dropDownMenu_int"] = data.dropDownMenu_int;
                result["dropDownMenu_prio0_str"] = QString::fromUtf8(data.dropDownMenu_prio0_str);
                result["dropDownMenu_prio1_str"] = QString::fromUtf8(data.dropDownMenu_prio1_str);
                result["dropDownMenu_prio2_str"] = QString::fromUtf8(data.dropDownMenu_prio2_str);
                result["dropDownMenu_prio3_str"] = QString::fromUtf8(data.dropDownMenu_prio3_str);
                result["dropDownMenu_prio4_str"] = QString::fromUtf8(data.dropDownMenu_prio4_str);

                return result;
            }
#endif
            case SETTINGS_ID_DISPLAY_SETTINGS: { // <-- new case for getting data for display settings
                DisplaySettingsData data;
                int size = sizeof(DisplaySettingsData);

                if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsRead((SETTINGS_ID_e)SETTINGS_ID_DISPLAY_SETTINGS, &data, &size)) {
                    return result;
                }

                result["selectedPanel"] = QString::fromUtf8(data.selectedPanel);
                return result;

            }

            default: {
                qDebug() << "Invalid ID";
                return result;
            }
        }
        return result;
    }

    Q_INVOKABLE void deleteData(int id) {
        SynaSettingsFramework settingsFramework;

        // Switch case for different settings IDs
        switch (id) {
            case SETTINGS_ID_DISPLAY_SETTINGS: {
                // Call the function to delete display settings
                if (SETTINGS_ERROR_NONE != settingsFramework.synaSettingsDelete(SETTINGS_ID_DISPLAY_SETTINGS)) {
                    qDebug() << "Failed to delete Display Settings.";
                }

                break;
            }
            // Add more cases if you have other settings IDs to delete
            default:
                qDebug() << "Invalid or unsupported settings ID: ";
                break;
        }
    }
};

#endif // SETTINGS_CONTROL_H
