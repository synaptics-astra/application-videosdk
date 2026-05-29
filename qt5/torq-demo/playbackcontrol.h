// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PLAYBACK_CONTROL_H
#define PLAYBACK_CONTROL_H

#include <QWidget>

#include "windowmanager.h"

class PlaybackControl : public QWidget {
    Q_OBJECT

    public:
        explicit PlaybackControl(string machine);
        ~PlaybackControl();
        Q_INVOKABLE void doPlayback(int index, QString url1, QString url2 = "",
                QString url3 = "", QString url4 = "");

        string machine;

    private:
        WindowManager* windowManager;
};

#endif // PLAYBACK_CONTROL_H
