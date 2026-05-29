// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef PLAYBACK_CONTROL_H
#define PLAYBACK_CONTROL_H

#include <QWidget>
#include <string>

#include "windowmanager.h"
#include "settingsmanager.h"

class PlaybackControl : public QWidget {
    Q_OBJECT

    public:
        explicit PlaybackControl(string machine);
        ~PlaybackControl();
        Q_INVOKABLE void doPlayback(int index, QString url1, QString url2,
                                    QString url3, QString url4);
        Q_INVOKABLE void launch(QString path);

        std::string machine;

    private:
        WindowManager* windowManager;
};

#endif // PLAYBACK_CONTROL_H
