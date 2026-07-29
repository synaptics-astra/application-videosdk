// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include <QImage>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QElapsedTimer>
#include <QMutex>
#include <atomic>
#include <memory>

class GstPlayer : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GstPlayer(QWidget *parent = nullptr);
    ~GstPlayer() override;

    bool startPlayback();
    void stopPlayback();
    GstElement *getOverlayElement() const { return overlayElement; }

private slots:
    void updateFrame();

private:
    static GstFlowReturn newBuffer(GstAppSink *appsink, gpointer userData);
    void cleanupPipeline();
    void releaseTexture();

    GstElement *pipeline = nullptr;
    GstElement *overlayElement = nullptr;
    GstAppSink *appsink = nullptr;
    QImage videoFrame;
    QImage pendingFrame;
    QMutex frameMutex;
    std::atomic_bool framePending { false };
    std::atomic_bool updateQueued { false };
    std::unique_ptr<QOpenGLTexture> frameTexture;
    QOpenGLShaderProgram shaderProgram;
    bool textureDirty = false;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
};

#endif // GSTPLAYER_H
