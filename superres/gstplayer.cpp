// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "gstplayer.h"

GstPlayer::GstPlayer(QWidget* parent) : QWidget(parent), pipeline(nullptr), sink(nullptr), surface(nullptr)
{
    QLabel* label = new QLabel("Video Will Play Here", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; font-weight: bold;");
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    command = new Command();
    setAttribute(Qt::WA_TransparentForMouseEvents);
    gst_init(NULL, NULL); // Initialize GStreamer
}

GstPlayer::~GstPlayer()
{
    cleanupPipeline();
    delete command;
}

void GstPlayer::startPlayback()
{
    cleanupPipeline(); // Clean up any existing pipeline

    GError* error = NULL;
    QString url = command->createPipeline();
    std::string pipelineStr = url.toUtf8().constData();
    pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
    if (!pipeline || error) {
        qCritical() << "Failed to create pipeline: " << (error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        if (pipeline) gst_object_unref(pipeline);
        pipeline = nullptr;
        sink = nullptr;
        return;
    }

    sink = (GstVideoOverlay*)gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (!sink || !GST_IS_VIDEO_OVERLAY(sink)) {
        qCritical() << "Failed to get valid sink from pipeline";
        cleanupPipeline();
        return;
    }

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    if (bus) {
        gst_bus_enable_sync_message_emission(bus);
        gst_bus_set_sync_handler(bus, gstBusCallback, this, NULL);
        gst_object_unref(bus);
    } else {
        qWarning() << "Failed to get pipeline bus";
    }

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to set pipeline to PLAYING state";
        cleanupPipeline();
        return;
    }

    QTimer::singleShot(100, [this]() {
        if (sink && GST_IS_VIDEO_OVERLAY(sink)) {
            gst_video_overlay_set_render_rectangle(sink, x(), y(), width(), height());
        }
    });
}

void GstPlayer::stopPlayback()
{
    cleanupPipeline();
}

void GstPlayer::cleanupPipeline()
{
    if (pipeline && GST_IS_ELEMENT(pipeline)) {
        GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_NULL);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qWarning() << "Failed to set pipeline to NULL state";
        }
        gst_object_unref(pipeline);
        pipeline = nullptr;
    } else if (pipeline) {
        qWarning() << "Pipeline is not a valid GstElement, cleaning up";
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }

    if (sink && GST_IS_VIDEO_OVERLAY(sink)) {
        gst_object_unref(sink);
        sink = nullptr;
    } else if (sink) {
        qWarning() << "Sink is not a valid GstVideoOverlay, cleaning up";
        gst_object_unref(sink);
        sink = nullptr;
    }
}

void GstPlayer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (sink && GST_IS_VIDEO_OVERLAY(sink)) {
        gst_video_overlay_set_render_rectangle(sink, x(), y(), width(), height());
        wl_surface_commit(surface);
    }
}

GstBusSyncReply GstPlayer::gstBusCallback(GstBus* bus, GstMessage* message, gpointer user_data)
{
    GstPlayer* self = static_cast<GstPlayer*>(user_data);

    if (gst_is_wl_display_handle_need_context_message(message)) {
        QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
        struct wl_display* display_handle = (struct wl_display*)native->nativeResourceForWindow("display", NULL);
        GstContext* context = gst_wl_display_handle_context_new(display_handle);
        gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)), context);
        gst_context_unref(context);
        goto drop;
    } else if (gst_is_video_overlay_prepare_window_handle_message(message)) {
        if (self->surface) {
            gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), (guintptr)self->surface);
            gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), self->x(), self->y(), self->width(), self->height());
            wl_surface_commit(self->surface);
        }
        goto drop;
    }

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_EOS:
            qDebug() << "End of stream reached";
            break;
        default:
            break;
    }

    return GST_BUS_PASS;

drop:
    gst_message_unref(message);
    return GST_BUS_DROP;
}

void GstPlayer::init(struct wl_surface* surface)
{
    this->surface = surface;
}

void GstPlayer::srEnable(bool enable)
{
    command->srEnable(enable);
    stopPlayback();
    startPlayback();
}
