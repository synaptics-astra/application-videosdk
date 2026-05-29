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
#ifdef HAVE_WAYLAND
#include <gst/wayland/wayland.h>
#endif

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

signals:
    void videoInfoChanged(const QString &resolution, float fps);
    void resolutionChanged(const QString &prevResolution, const QString &currResolution);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    static GstBusSyncReply gstBusCallback(GstBus* bus, GstMessage* message, gpointer user_data);
    void cleanupPipeline();
    void updateVideoInfo();
    bool configurePlatformSpecificOverlay();

    GstElement* pipeline;
    GstVideoOverlay* sink;
    struct wl_surface* surface;
    QTimer* videoInfoTimer;
    int lastWidth;
    int lastHeight;
    float lastFps;
};

#endif // GSTPLAYER_H