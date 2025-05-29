// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include "command.h"
#include "embedding.h"

#include <glib.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include <string>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QImage>
#include <QPainter>

typedef struct _GstSynapMeta GstSynapMeta;

typedef enum
{
    GST_SYNAP_POSTPROCESSING_MODE_NONE = 0,               /**< Mode None */
    GST_SYNAP_POSTPROCESSING_MODE_CLASSIFIER = 1,         /**< Classification Mode */
    GST_SYNAP_POSTPROCESSING_MODE_DETECTOR = 2,           /**< Detection Mode */
    GST_SYNAP_POSTPROCESSING_MODE_RECOGNITION = 3,        /**< Recognition Mode */
} GstSynapPostProcessingMode;

typedef struct {
    GstMeta meta;
    gpointer data;
    gint width;
    gint height;
    gfloat confthreshold;
} GstClassifierMeta;

typedef struct {
    GstMeta meta;
    gpointer data;
    gint width;
    gint height;
} GstDetectorMeta;

typedef struct {
    GstMeta meta;
    gpointer data;
    gint tlx;
    gint tly;
    gint brx;
    gint bry;
    gint width;
    gint height;
} GstRecognitionMeta;

struct _GstSynapMeta {
    GstSynapPostProcessingMode mode;
    union {
        GstClassifierMeta classifier;
        GstDetectorMeta detector;
        GstRecognitionMeta recognition;
    } type;
};

class Command;
class Embedding;
class GstPlayer: public QWidget {
    Q_OBJECT
    public:
        GstPlayer(QWidget* parent);
        ~GstPlayer();

        bool startPlayback();
        void stopPlayback();
        void setFaceName(QString name, cv::Mat m);
        void updateImage(const QImage& image);

    signals:
        void faceDetected(QImage image, cv::Mat m);
        void pushFrame(QImage image);

    private:
        static GstFlowReturn newBuffer(GstAppSink *appsink, gpointer userData);

        GstElement* pipeline;
        GstAppSink *appsink;
        QImage videoFrame;
        Command *command;
        Embedding *embedding;

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void paintEvent(QPaintEvent* event) override;
};

#endif // GSTPLAYER_H
