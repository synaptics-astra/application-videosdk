// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "playbackcontrol.h"

void PlaybackControl::createManager()
{
    windowManager = new WindowManager();
}

void PlaybackControl::doPlayback(int index, QString url1, QString url2,
        QString url3, QString url4)
{
    windowManager->play(index, url1, url2, url3, url4);
    windowManager->show();
}
