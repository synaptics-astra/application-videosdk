// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "playerui.h"

int main(int argc, char** argv)
{
    QApplication* app;
    PlayerUI* playerui;
    gst_init(0, 0);

    app = new QApplication(argc, argv);
    playerui = new PlayerUI(0);

    playerui->show();
    return app->exec();
}

