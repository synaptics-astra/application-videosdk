// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef PLAYBACK_CONTROL_H
#define PLAYBACK_CONTROL_H

#include <QWidget>

#include "windowmanager.h"
#include "settingsmanager.h"

class PlaybackControl : public QWidget {
    Q_OBJECT

    public:
        PlaybackControl();
        ~PlaybackControl();
        Q_INVOKABLE void doPlayback(int index, QString url1, QString url2,
                QString url3, QString url4);
        Q_INVOKABLE void launch(QString path);
        Q_INVOKABLE QString getMachine() { return QString(machine.c_str()); }

        string machine;

    private:
        WindowManager* windowManager;
};

#endif // PLAYBACK_CONTROL_H
