// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <signal.h>
#include <unistd.h>

#include "playerui.h"

QApplication* appPtr = nullptr;

void sigintHandler(int) {
    if (appPtr) {
        appPtr->quit();
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    appPtr = &app;
    signal(SIGINT, sigintHandler);

    PlayerUI playerui(0);
    gst_init(0, 0);
    playerui.showFullScreen();
    return app.exec();
}
