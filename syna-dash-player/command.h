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
#include <QProcess>

#include "videoprober.h"

class Command : public QObject {
    Q_OBJECT

public:
    Command();
    ~Command();
    QString createPipeline(int widgetWidth, int widgetHeight);
    bool isResolutionSupported(int &videoWidth, int &videoHeight);
    void setInputType(QString inputType);
    void setInputSource(QString inputSource);
    QString getInputType() const { return inputType; }
    QString getMachine();
    QString machine;

private:
    QString inputType;
    QString inputSource;
    QString pipeline;
};

#endif // COMMAND_H