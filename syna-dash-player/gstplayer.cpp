// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "gstplayer.h"
#include <gst/app/gstappsink.h>
#include <QPainter>

GstPlayer::GstPlayer(QWidget* parent)
    : QWidget(parent)
    , command(new Command())
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setStyleSheet("background: black;");
    connect(this, &GstPlayer::pushFrame, this, &GstPlayer::updateImage);
}

GstPlayer::~GstPlayer()
{
    stopPlayback();
    delete command;
}

void GstPlayer::startPlayback()
{
    cleanupPipeline();

    GError* err = nullptr;
    QString pipelineStr = command->createPipeline(width(), height());
    pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &err);
    if (!pipeline || err) {
        qCritical() << "Pipeline failed:" << (err ? err->message : "unknown");
        if (err) g_error_free(err);
        return;
    }

    appsink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "sink"));
    if (!appsink) {
        qCritical() << "No appsink in pipeline";
        cleanupPipeline();
        return;
    }

    GstAppSinkCallbacks cb = { nullptr };
    cb.new_sample = newBuffer;
    gst_app_sink_set_callbacks(appsink, &cb, this, nullptr);
    gst_app_sink_set_drop(appsink, TRUE);
    gst_app_sink_set_max_buffers(appsink, 1);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void GstPlayer::stopPlayback()
{
    cleanupPipeline();
    videoFrame = QImage();
    update();
}

void GstPlayer::cleanupPipeline()
{
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
    if (appsink) {
        gst_object_unref(appsink);
        appsink = nullptr;
    }
}

void GstPlayer::updateImage(const QImage& img)
{
    videoFrame = img;
    update();
}

void GstPlayer::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if (!videoFrame.isNull()) {
        QImage s = videoFrame.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawImage((width() - s.width()) / 2, (height() - s.height()) / 2, s);
    } else {
        p.fillRect(rect(), Qt::black);
    }
}

GstFlowReturn GstPlayer::newBuffer(GstAppSink* sink, gpointer userData)
{
    GstPlayer* self = static_cast<GstPlayer*>(userData);
    GstSample* sample = gst_app_sink_pull_sample(sink);
    if (!sample) return GST_FLOW_ERROR;

    GstBuffer* buf = gst_sample_get_buffer(sample);
    GstCaps* caps = gst_sample_get_caps(sample);

    if (buf && caps) {
        GstMapInfo map;
        if (gst_buffer_map(buf, &map, GST_MAP_READ)) {
            GstStructure* s = gst_caps_get_structure(caps, 0);
            gint w = 0, h = 0;
            gst_structure_get_int(s, "width", &w);
            gst_structure_get_int(s, "height", &h);

            QImage img(map.data, w, h, QImage::Format_RGB888);
            if (!img.isNull()) {
                emit self->pushFrame(img.copy());

                if (w != self->lastWidth || h != self->lastHeight) {
                    QString prev = self->lastWidth > 0 ? QString("%1x%2").arg(self->lastWidth).arg(self->lastHeight) : "N/A";
                    QString curr = QString("%1x%2").arg(w).arg(h);
                    self->lastWidth = w;
                    self->lastHeight = h;
                    emit self->resolutionChanged(prev, curr);
                    emit self->videoInfoChanged(curr, self->lastFps);
                }

                const GValue* fv = gst_structure_get_value(s, "framerate");
                if (fv && GST_VALUE_HOLDS_FRACTION(fv)) {
                    int n = gst_value_get_fraction_numerator(fv);
                    int d = gst_value_get_fraction_denominator(fv);
                    float fps = d ? (float)n / d : 0.0f;
                    if (qAbs(fps - self->lastFps) > 0.1f) {
                        self->lastFps = fps;
                        emit self->videoInfoChanged(QString("%1x%2").arg(w).arg(h), fps);
                    }
                }
            }
            gst_buffer_unmap(buf, &map);
        }
    }
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}