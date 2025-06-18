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
        emit availablePanelsUpdated(modelData);
    }
}

QString DisplaySettings::extractPanelName(const QString &fileName) {
    QString panelName = fileName;
    panelName.remove(0, panelName.indexOf('-') + 1);
    panelName.chop(QString("-overlay.dtbo").length());
    return panelName;
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
         if (file.endsWith(panelFileName + "-overlay.dtbo")) {
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
    QFile configFile(WESTON_CONFIG_PATH);

    if (!configFile.exists()) {
        emit statusUpdated("Reset to default configuration. Reboot required. (Weston config file not found.)");
        return;
    }

    if (!configFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        emit statusUpdated("Reset to default configuration. Reboot required. (Failed to access Weston config file.)");
        return;
    }

    QStringList lines;
    QTextStream in(&configFile);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    // Remove DPI-2 section
    removeDpi2Section(lines);

    if (!configFile.seek(0)) {
        configFile.close();
        emit statusUpdated("Reset to default configuration. Reboot required. (Failed to seek in Weston config file.)");
        return;
    }

    QTextStream out(&configFile);
    for (const QString &line : lines) {
        out << line << "\n";
    }

    qint64 newSize = configFile.pos();
    if (!configFile.resize(newSize)) {
        configFile.close();
        emit statusUpdated("Reset to default configuration. Reboot required. (Failed to resize Weston config file.)");
        return;
    }

    configFile.close();
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

    QString output = process.readAllStandardOutput().trimmed();
    QStringList parts = output.split("=");
    QString currentPanel = (parts.size() > 1) ? parts[1].trimmed() : QString();

    if (currentPanel.isEmpty()) {
        emit statusUpdated("No panel is selected.");
        return;
    }

    emit statusUpdated(QString("Selected Panel: %1").arg(extractPanelName(currentPanel)));
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
    QStringList panelNames;

    for (const QString &filePath : dtboFiles) {
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        dtboFiles.replaceInStrings("/boot/", "");
        // filter out files containing "panel"
        if (!fileName.contains("panel")) {
            continue; //skip non-panel files
        }
        panelNames.append(extractPanelName(fileName));
    }

    return panelNames;
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

bool DisplaySettings::removeDpi2Section(QStringList &lines) {
    int dpi2StartIndex = -1;
    int dpi2EndIndex = -1;
    bool foundDPI2Section = false;
    bool insideOutputSection = false;

    // Parse to find DPI-2 section
    for (int i = 0; i < lines.size(); i++) {
        QString trimmedLine = lines[i].trimmed();
        if (trimmedLine.startsWith("[output]")) {
            insideOutputSection = true;
            for (int j = i + 1; j < lines.size() && !lines[j].trimmed().startsWith("["); j++) {
                if (lines[j].trimmed().startsWith("name=DPI-2")) {
                    dpi2StartIndex = i;
                    foundDPI2Section = true;
                    break;
                }
            }
        } else if (insideOutputSection && trimmedLine.startsWith("[")) {
            insideOutputSection = false;
            if (foundDPI2Section && dpi2EndIndex == -1) {
                dpi2EndIndex = i;
            }
        }
    }

    if (foundDPI2Section && dpi2EndIndex == -1) {
        dpi2EndIndex = lines.size();
    }

    // Remove DPI-2 section if found
    if (foundDPI2Section) {
        if (dpi2StartIndex >= 0 && dpi2StartIndex < lines.size() &&
            dpi2EndIndex > dpi2StartIndex && dpi2EndIndex <= lines.size()) {
            lines.erase(lines.begin() + dpi2StartIndex, lines.begin() + dpi2EndIndex);
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void DisplaySettings::modifyWestonConfig(const QString &selectedPanel) {
    QFile configFile(WESTON_CONFIG_PATH);
    // Get the system profile by running `uname -n`
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << "uname -n");
    if (!process.waitForFinished()) {
        emit statusUpdated("Failed to fetch system profile.");
        return;
    }

    QString systemProfile = process.readAllStandardOutput().trimmed();

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

    bool isHaierPanel = selectedPanel.contains("haier");
    bool isSL1620 = (systemProfile == "sl1620");
    bool insideOutputSection = false;
    bool transformLineFound = false;
    int transformLineIndex = -1;
    int outputSectionIndex = -1;

    // Track DPI-2 section (still needed for detection in Case 1)
    int dpi2StartIndex = -1;
    int dpi2EndIndex = -1;
    bool foundDPI2Section = false;

    // Parse config file to find necessary sections
    for (int i = 0; i < lines.size(); i++) {
        QString trimmedLine = lines[i].trimmed();

        if (trimmedLine.startsWith("[output]")) {
            insideOutputSection = true;
            outputSectionIndex = i;

            // Look ahead for name=DPI-2 within this section
            for (int j = i + 1; j < lines.size() && !lines[j].trimmed().startsWith("["); j++) {
                if (lines[j].trimmed().startsWith("name=DPI-2")) {
                    dpi2StartIndex = i;
                    foundDPI2Section = true;
                    break;
                }
            }
        } else if (insideOutputSection && trimmedLine.startsWith("[")) {
            insideOutputSection = false;
            if (foundDPI2Section && dpi2EndIndex == -1) {
                dpi2EndIndex = i;
            }
        }

        if (insideOutputSection && trimmedLine.startsWith("transform=rotate-270")) {
            transformLineFound = true;
            transformLineIndex = i;
        }
    }

    if (foundDPI2Section && dpi2EndIndex == -1) {
        dpi2EndIndex = lines.size();
    }

    // Case 1: If profile is sl1620 and panel is Haier, ensure DPI-2 section exists
    if (isSL1620 && isHaierPanel) {
        if (!foundDPI2Section) {
            if (!lines.isEmpty() && !lines.last().isEmpty()) {
                lines.append("");
            }
            lines.append("[output]");
            lines.append("name=DPI-2");
            lines.append("transform=rotate-270");
            lines.append("# Added by DisplaySettings");
        }
    }
    // Case 2: If profile is sl1620 and panel is NOT Haier, remove DPI-2 section
    else if (isSL1620 && !isHaierPanel && foundDPI2Section) {
        removeDpi2Section(lines);
    }
    // Case 3: If profile is NOT sl1620 but panel is Haier, add transform to existing [output]
    else if (!isSL1620 && isHaierPanel && !transformLineFound && outputSectionIndex != -1) {
        lines.insert(outputSectionIndex + 1, "transform=rotate-270");
    }
    // Case 4: If profile is NOT sl1620 and panel is NOT Haier, remove transform if found
    else if (!isSL1620 && !isHaierPanel && transformLineFound) {
        lines.removeAt(transformLineIndex);
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

    emit statusUpdated("Weston configuration updated successfully.");
}
