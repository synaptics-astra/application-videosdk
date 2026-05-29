// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QImage>
#include <QStringList>
#include <QMutex>
#include <QString>
#include <QVector>

#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "qtdevice.h"

class CameraProcessor : public QObject {
    Q_OBJECT

public:
    CameraProcessor();
    ~CameraProcessor();

    bool start(QString pipeline);
    bool readFrame(QImage* outImage);
    void stop();

    QString getCamNode(QString inputSource);
    QtDevice* qtDevice = new QtDevice();
    QString width = "640";
    QString height = "480";
    QVector<QVector<std::string>> qtDevices;
    QString v4l2src;
    QString inputSource;

signals:
    void frameReady(const QImage &image);

private:
    static GstFlowReturn onNewSample(GstAppSink* appSink, gpointer userData);
    GstFlowReturn processSample(GstSample* sample);
    void loadLabelsFromResource();
    QString labelForClass(int classIndex) const;

    GstElement* pipeline = nullptr;
    GstAppSink* appSink = nullptr;
    QImage latestFrame;
    QMutex frameMutex;
    QStringList classLabels;
};

#endif
