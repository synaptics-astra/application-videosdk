// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "playbackcontrol.h"

PlaybackControl::PlaybackControl()
{
    windowManager = new WindowManager();
}

PlaybackControl::~PlaybackControl()
{
    delete windowManager;
}

void PlaybackControl::doPlayback(QString url1, QString url2)
{
    windowManager->show();
    windowManager->play(url1, url2);
}
