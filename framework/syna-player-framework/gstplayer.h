// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef GST_PLAYER_H
#define GST_PLAYER_H

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <qpa/qplatformnativeinterface.h>
#ifdef HAVE_WAYLAND
#include <gst/wayland/wayland.h>
#endif

#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QWidget>

#include "windowmanager.h"

using namespace std;

class WindowManager;
class GstPlayer: public QThread {
    Q_OBJECT
    public:
        void run() override;
        GstPlayer (int index, int x, int y, int width, int height,
                QString url, WindowManager *windowManager);
        ~GstPlayer() {}
        static GstBusSyncReply gstBusCallback(GstBus* bus, GstMessage* message,
                gpointer user_data);

    public slots:
        void stopThread();

    private:
        int index, x, y, width, height;
        string pipelineStr;
        const char* cmd;
#ifdef HAVE_WAYLAND
        struct wl_surface* surface;
#endif

        QString url;
        GstElement* pipeline;
        GstVideoOverlay* sink;
        WindowManager* windowManager;
        bool configurePlatformSpecificOverlay();
        int is_valid_fd[FD_SETSIZE];

        int list_fds();
        int check_and_close_fds();
};
#endif // GST_PLAYER_H
