// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class CameraImageProvider;

class CameraManager : public QObject {
    Q_OBJECT
public:
    explicit CameraManager(CameraImageProvider *imageProvider, QObject *parent = nullptr);
    ~CameraManager();

    Q_INVOKABLE void launchCamera();
    Q_INVOKABLE void stopCamera();
    Q_INVOKABLE void captureImage();

signals:
    void previewUpdated();
    void captureResult(bool success, QString message);
    void noCameraDetected(QString message);
    void previewStarted();

private:
    QString getCameraPath();
    void setupPreviewPipeline();
    static GstFlowReturn newSampleCallback(GstAppSink *sink, gpointer user_data);

    GstElement *previewPipeline;
    GstAppSink *appSink;
    GstBus *pipelineBus;
    CameraImageProvider *previewImageProvider;
    QString devicePath;
    bool isCapturing;
};

#endif // CAMERAMANAGER_H