// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include <QQuickImageProvider>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QMutex>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class FrameProvider : public QObject, public QQuickImageProvider {
    Q_OBJECT
public:
    FrameProvider(QObject *parent = nullptr);
    ~FrameProvider();

    Q_INVOKABLE void startWithDefaultFiles();
    Q_INVOKABLE void addUserFiles(const QVariantList& fileUrls);
    Q_INVOKABLE void removeUserFiles(const QVariantList& fileUrls);
    Q_INVOKABLE void playUserFiles();
    Q_INVOKABLE void playSingleFile(const QString &filePath);
    Q_INVOKABLE void clear();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

signals:
    void imageChanged();

private:
    void init();
    void createPipeline();
    void extractHalfFrame(const uint8_t *originalFrame, int originalWidth, int originalHeight, uint8_t *halfFrame, bool left);
    void updateImage(uint8_t *data, int len);
    static GstFlowReturn newBuffer(GstAppSink *appsink, gpointer user_data);

    QString modelPath;
    QString gstPipeline;
    QString gstPipelineTemplate;
    QStringList files;
    int currentIndex;
    uint8_t *leftData;
    uint8_t *rightData;
    GstElement *pipeline;
    GstAppSink *appsink_1;
    GstAppSink *appsink_2;
    QMutex mutex_;
    int callbackCount;
    bool normalFrameProcessed;
    bool srFrameProcessed;
};

#endif // FRAMEPROVIDER_H