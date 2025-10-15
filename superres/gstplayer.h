// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include "command.h"
#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QDebug>
#include <string>
#include <QLabel>
#include <QVBoxLayout>

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <qpa/qplatformnativeinterface.h>
#include <gst/wayland/wayland.h>

class Command;
class GstPlayer : public QWidget {
    Q_OBJECT
public:
    explicit GstPlayer(QWidget* parent = nullptr);
    ~GstPlayer();
    void init(struct wl_surface* surface);
    void startPlayback();
    void stopPlayback();
    Command *command;

public slots:
    void srEnable(bool enable);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    static GstBusSyncReply gstBusCallback(GstBus* bus, GstMessage* message, gpointer user_data);
    void cleanupPipeline();

    GstElement* pipeline;
    GstVideoOverlay* sink;
    struct wl_surface* surface;
};

#endif // GSTPLAYER_H
