// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#include <signal.h>
#include <cstdlib>

#include <QApplication>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

#include "gstplayer.h"
#include "overlaytestpanel.h"

QApplication *appPtr = nullptr;

void sigintHandler(int)
{
    if (appPtr) {
        appPtr->quit();
    }
}

int main(int argc, char **argv)
{
    setenv("LC_ALL", "C.UTF-8", 0);
    setenv("LANG", "C.UTF-8", 0);

    QApplication app(argc, argv);
    appPtr = &app;
    signal(SIGINT, sigintHandler);

    gst_init(nullptr, nullptr);

    QWidget window;
    window.setWindowTitle("synapoverlay-test");
    window.resize(1400, 800);

    auto *mainLayout = new QVBoxLayout(&window);

    auto *splitter = new QSplitter(Qt::Horizontal);
    auto *player = new GstPlayer();
    auto *panel = new OverlayTestPanel(player);

    player->setMinimumSize(800, 500);
    splitter->addWidget(player);
    splitter->addWidget(panel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);
    QObject::connect(&app, &QApplication::aboutToQuit, player, &GstPlayer::stopPlayback);

    if (!player->startPlayback()) {
        qCritical() << "Unable to start playback pipeline";
    }

    window.showMaximized();
    int rc = app.exec();
    gst_deinit();
    return rc;
}
