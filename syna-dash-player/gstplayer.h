// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include "command.h"
#include <QWidget>
#include <QImage>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class GstPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit GstPlayer(QWidget* parent = nullptr);
    ~GstPlayer();

    void startPlayback();
    void stopPlayback();

    Command* command = nullptr;

signals:
    void pushFrame(const QImage& image);
    void videoInfoChanged(const QString& resolution, float fps);
    void resolutionChanged(const QString& prev, const QString& curr);

protected:
    void paintEvent(QPaintEvent*) override;

private slots:
    void updateImage(const QImage& image);

private:
    static GstFlowReturn newBuffer(GstAppSink* sink, gpointer userData);
    void cleanupPipeline();

    GstElement* pipeline = nullptr;
    GstAppSink* appsink = nullptr;
    QImage videoFrame;

    int lastWidth = -1;
    int lastHeight = -1;
    float lastFps = -1.0f;
};

#endif