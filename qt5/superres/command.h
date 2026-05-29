// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef COMMAND_H
#define COMMAND_H

#include <QString>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <gst/gst.h>

#include "videoprober.h"
#include "qtdevice.h"

class Command : public QObject {
    Q_OBJECT

public:
    Command();
    ~Command();
    QString createPipeline();
    QString getCamNode(QString inputSource);
    bool isResolutionSupported(int &videoWidth, int &videoHeight);
    void setInputType(QString inputType);
    void setInputSource(QString inputSource);
    void setUpscalingType(QString upscalingType);
    void setModelType(QString modelType);
    void srEnable(bool enable);

private:
    QString inputType;
    QString inputSource;
    QString upscalingType;
    QString modelType;
    QString width;
    QString height;
    QString v4l2src;
    QString pipeline;
    QtDevice *qtDevice;
    QVector<QVector<std::string>> qtDevices;
    bool srEnabled;
};

#endif // COMMAND_H
