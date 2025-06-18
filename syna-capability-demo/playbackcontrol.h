// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef PLAYBACK_CONTROL_H
#define PLAYBACK_CONTROL_H

#include <QWidget>

#include "windowmanager.h"
#include "filecontrol.h"

class PlaybackControl : public QWidget {
    Q_OBJECT

    public:
        PlaybackControl();
        ~PlaybackControl();
        Q_INVOKABLE void doPlayback(QString url1, QString url2);

        string machine;

    private:
        WindowManager* windowManager;
};

#endif // PLAYBACK_CONTROL_H
