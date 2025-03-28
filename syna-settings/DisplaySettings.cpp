// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "DisplaySettings.h"
#include <QDebug>
#include <QFileInfo>
#include <QProcess>

#define WESTON_CONFIG_PATH "/etc/xdg/weston/weston.ini"

DisplaySettings::DisplaySettings(QObject *parent) : QObject(parent) {}

void DisplaySettings::fetchAvailablePanels() {
    QStringList availablePanels = getDtbFiles();

    if (availablePanels.isEmpty()) {
        emit statusUpdated("No .dtbo files found in /boot.");
    } else {
        QVariantList modelData;
        int panelNumber = 1;
        for (const QString &panel : availablePanels) {
            QVariantMap panelItem;
            panelItem["text"] = QString("Panel %1 - %2").arg(panelNumber).arg(panel); //  Show "Panel X - filename.dtbo"
            panelItem["value"] = panel;
            modelData.append(panelItem);
            panelNumber++;
        }
        emit availablePanelsUpdated(modelData);  //  Send correctly formatted data
    }
}

//save selected panel and store it consitently
void DisplaySettings::savePanel(const QString &panelFileName) {
    if (panelFileName.isEmpty()) {
        emit statusUpdated("No panels selected.");
        return ;
    }

    if (dtboFiles.isEmpty()) {
        emit statusUpdated("Error: No DTBO files available. Fetch panels first.");
        return;
    }

    QString dtboFile;

    for (const QString &file : dtboFiles) {
         if ((panelFileName.contains("Haier") && file.contains("haier-panel-overlay")) ||
             (panelFileName.contains("Waveshare") && file.contains("ws-panel-overlay"))) {
             dtboFile = file;
             break;
          }
    }

    if (dtboFile.isEmpty()) {
        emit statusUpdated("Error: No matching .dtbo file found for the selected panel");
        return;
    }
//apply panel to u-boot
    QString command = QString("fw_setenv dtbo %1").arg(dtboFile);
    executeCommand(command);
    modifyWestonConfig(panelFileName);
    emit statusUpdated(QString("'%1' saved and applied.").arg(panelFileName));
}

//Reset to default settings (removes stored settings and resets U-boot panel)
void DisplaySettings::resetToDefault() {
    executeCommand("fw_setenv dtbo");
    emit statusUpdated("Reset to default configuration. Reboot required.");
}

void DisplaySettings::printPanelInfo() {
    QProcess process;
    process.start("fw_printenv", QStringList() << "dtbo");

    if (!process.waitForFinished()) {
        emit statusUpdated("Failed to fetch the panel info.");
        return;
    }

    QString currentPanel = process.readAllStandardOutput().trimmed();
    if (currentPanel.isEmpty()) {
        emit statusUpdated("No panel is selected.");
    } else if (currentPanel.contains("haier-panel-overlay")) {
        emit statusUpdated("Selected Panel: Haier.");
    } else if (currentPanel.contains("ws-panel-overlay")) {
        emit statusUpdated("Selected Panel: Waveshare.");
    } else {
        emit statusUpdated("No panel selected.");
    }
}

//Get the list of available DTB files from /boot
QStringList DisplaySettings::getDtbFiles() {
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << "ls /boot/*.dtbo");

    if (!process.waitForFinished()) {
        emit statusUpdated("Failed to fetch dtbo files");
        return {};
    }

    QString output = process.readAllStandardOutput().trimmed();

    if (output.isEmpty()) {
        emit statusUpdated("No .dtbo files found in /boot");
        return {};
    }

    dtboFiles = output.split("\n", Qt::SkipEmptyParts);
    QStringList filteredFiles;

    for (const QString &filePath : dtboFiles) {
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        dtboFiles.replaceInStrings("/boot/", "");
        if (fileName.contains("haier-panel-overlay")) {
            filteredFiles.append("Haier Panel");
        } else if (fileName.contains("ws-panel-overlay")) {
            filteredFiles.append("Waveshare Panel");
        }
    }

    return filteredFiles;
}

//execute shell command
void DisplaySettings::executeCommand(const QString &command, bool emitOutput) {
    QProcess process;
    process.start("sh", QStringList() << "-c" << command);

    if (!process.waitForFinished()) {
        emit statusUpdated("Command execution failed or timeout");
        return;
    }

    QString stdoutOutput = process.readAllStandardOutput();
    QString stderrOutput = process.readAllStandardError();

    if (emitOutput) {
        if (!stdoutOutput.isEmpty()) {
            emit statusUpdated(stdoutOutput.trimmed());
        } else if (!stderrOutput.trimmed().isEmpty()) {
            emit statusUpdated("Error: " + stderrOutput.trimmed());
        }
    }
}

void DisplaySettings::modifyWestonConfig(const QString &selectedPanel) {
    QFile configFile(WESTON_CONFIG_PATH);
    qDebug() << "Selected Panel: " << selectedPanel;
    // Check if the Weston config file exists
    if (!configFile.exists()) {
        emit statusUpdated("Weston config file not found.");
        return;
    }

    // Try opening the file for reading
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit statusUpdated("Failed to read Weston config file.");
        return;
    }

    QStringList lines;
    QTextStream in(&configFile);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    configFile.close();

    bool isHaierPanel = selectedPanel.contains("Haier");
    bool isWavesharePanel = selectedPanel.contains("Waveshare");

    if (!isHaierPanel && !isWavesharePanel) {
        emit statusUpdated("No modification needed for Weston config.");
        return;
    }

    bool insideOutputSection = false;
    bool transformLineFound = false;
    int transformLineIndex = -1;

    // Check if the [output] section exists, if not, create it
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].trimmed().startsWith("[output]")) {
            insideOutputSection = true;
            break;
        }
    }

    // If [output] section doesn't exist, create it
    if (!insideOutputSection) {
        // Add [output] section before any other section or at the end if no other sections exist
        bool sectionInserted = false;
        for (int i = 0; i < lines.size(); i++) {
            if (lines[i].trimmed().startsWith("[")) {
                lines.insert(i, "[output]");
                sectionInserted = true;
                break;
            }
        }

        // If no section was inserted, append it at the end
        if (!sectionInserted) {
            lines.append("\n[output]");
        }
    }

    // Now that [output] section is ensured, check for transform=rotate-270 for Haier or Waveshare
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].contains("transform=rotate-270")) {
            transformLineFound = true;
            transformLineIndex = i;
            break;
        }
    }

    // For Haier panel: Add transform=rotate-270 if not found
    if (isHaierPanel) {
        if (!transformLineFound) {
            // Add transform=rotate-270 under the [output] section
            for (int i = 0; i < lines.size(); i++) {
                if (lines[i].trimmed().startsWith("[output]")) {
                    lines.insert(i + 1, "transform=rotate-270");
                    break;
                }
            }
        }
    }
    // For Waveshare panel: Remove transform=rotate-270 if found
    else if (isWavesharePanel) {
        if (transformLineFound) {
            lines.removeAt(transformLineIndex);
        }
    }

    // Write back the updated Weston configuration
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        emit statusUpdated("Failed to update Weston config file.");
        return;
    }

    QTextStream out(&configFile);
    for (const QString &line : lines) {
        out << line << "\n";
    }
    configFile.close();
}

