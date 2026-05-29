// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef DISPLAYSETTINGS_H
#define DISPLAYSETTINGS_H

#include<QObject>
#include<QStringList>
#include<QProcess>
#include "settingscontrol.h"

class DisplaySettings : public QObject {
    Q_OBJECT
public:
    explicit DisplaySettings(QObject *parent = nullptr);

    Q_INVOKABLE void fetchAvailablePanels ();
    Q_INVOKABLE void savePanel(const QString &panel);
    Q_INVOKABLE void resetToDefault ();
    Q_INVOKABLE void printPanelInfo ();
    Q_INVOKABLE void executeCommand(const QString &command, bool emitOutput = true);
#ifdef HAVE_WAYLAND
    void modifyWestonConfig(const QString &selectedPanel);
#endif // HAVE_WAYLAND
    QString extractPanelName(const QString &fileName);

signals:
    void availablePanelsUpdated(const QVariantList &panels);
    void statusUpdated(const QString &message);

private:
    QStringList getDtbFiles ();
    QStringList dtboFiles;
#ifdef HAVE_WAYLAND
    bool removeDpi2Section(QStringList &lines);
#endif // HAVE_WAYLAND
};

#endif // DISPLAYSETTINGS_H
