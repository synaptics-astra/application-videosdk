// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef APPSINK_H
#define APPSINK_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QSysInfo>
#include <QFile>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

class AppSink : public QObject {
    Q_OBJECT

public:
    explicit AppSink(QObject *parent = nullptr);
    ~AppSink();

    Q_INVOKABLE void init();
    Q_INVOKABLE void createPipeline();
    void stopPipeline();
    Q_INVOKABLE void readLabels(const QString &fileName);
    Q_INVOKABLE QString getcpuload();
    Q_INVOKABLE QString getgpuload();
    Q_INVOKABLE QString getnpuload();

signals:
    void sendString(const QString &string);

private:
    GstElement* pipeline;
    GstAppSink *appsink;

    QStringList labels;
    float confidenceThreshold;
    QString modelPath;
    QString labelPath;
    QString gstPipeline;

    static GstFlowReturn newBuffer(GstAppSink *appsink, gpointer userData);
    static void updateTopLabelIndex(guint8 *scores, guint len, AppSink* appsink);
};

#endif // APPSINK_H
