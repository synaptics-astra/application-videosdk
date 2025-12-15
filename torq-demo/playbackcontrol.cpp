// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "playbackcontrol.h"

PlaybackControl::PlaybackControl(string machine) : machine(machine)
{
    windowManager = new WindowManager(nullptr, this->machine, "Torq Demo");
}

PlaybackControl::~PlaybackControl()
{
    delete windowManager;
}

void PlaybackControl::doPlayback(int index, QString url1, QString url2,
                                QString url3, QString url4)
{
    windowManager->show();
    windowManager->play(index, url1, url2, url3, url4);
}
