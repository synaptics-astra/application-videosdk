// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QEventLoop>
#include <QMutex>
#include <QUrl>
#include <QDir>
#include <QObject>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class FrameProvider : public QObject, public QQuickImageProvider {
    Q_OBJECT

public:
    FrameProvider(QObject *parent = nullptr);
    ~FrameProvider();
    void init();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
    void extractHalfFrame(const uint8_t* originalFrame, int originalWidth, int originalHeight, uint8_t* halfFrame, bool left);

    Q_INVOKABLE void startWithDefaultFiles();
    Q_INVOKABLE void addUserFiles(const QVariantList& fileUrls);
    Q_INVOKABLE void removeUserFiles(const QVariantList& fileUrls);
    Q_INVOKABLE void playUserFiles();

private:
    uint8_t* leftData;
    uint8_t* rightData;
    int leftLen;
    int rightLen;
    int callbackCount;
    int currentIndex;

    bool normalFrameProcessed, srFrameProcessed;

    GstElement *pipeline, *filesrc;
    GstAppSink *appsink_1, *appsink_2;
    GstBuffer *buffer;
    GstElement *element;

    QString modelPath;
    QString gstPipeline;
    QString gstPipelineTemplate;
    QEventLoop loop;
    QStringList files;
    QMutex mutex_;

    static GstFlowReturn newBuffer(GstAppSink *appsink, gpointer user_data);

public slots:
    void createPipeline();
    void updateImage(uint8_t* data, int len);

signals:
    Q_DECL_EXPORT Q_SIGNAL void imageChanged();
};

#endif // FRAMEPROVIDER_H
