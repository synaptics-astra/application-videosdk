// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef COMMAND_H
#define COMMAND_H

#include <QDebug>
#include "qtdevice.h"

class Command {
    public:
        Command();
        ~Command();
        QString createPipeline();
        QString getCamNode(QString inputSource);

        QString url;
        QString inputSource;
        QString detectionModel, recognitionModel;
        QString width;
        QString height;
        QString v4l2src;
        QString inputType;
        bool urlemptyflag = true;

        QString pipeline;
        QtDevice *qtDevice;
        QVector<QVector<string>> qtDevices;
};

#endif // COMMAND_H
