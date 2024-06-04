// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "gstplayer.h"

GstBusSyncReply GstPlayer::gstBusCallback(GstBus* bus, GstMessage* message,
        gpointer user_data)
{
    GstPlayer* player = static_cast<GstPlayer*>(user_data);
    if(1 && gst_is_wl_display_handle_need_context_message(message)){
        gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC (message)),
                player->windowManager->context);
        goto drop;
    }
    else if(1 && gst_is_video_overlay_prepare_window_handle_message(message)){
        QPlatformNativeInterface* native =
            QGuiApplication::platformNativeInterface();
        struct wl_surface* surface =
            static_cast<struct wl_surface*>(native->nativeResourceForWindow("surface",
                        player->windowManager->windowHandle()));
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)),
                (guintptr)surface);
        gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)),
                player->x, player->y, player->width, player->height);
        goto drop;
    }

    return GST_BUS_PASS;

drop:
    gst_message_unref(message);
    return GST_BUS_DROP;
}

GstPlayer::GstPlayer (int index, int x, int y, int width, int height,
        QString url, WindowManager *windowManager)
    : index(index), x(x), y(y), width(width), height(height), url(url),
    windowManager(windowManager)
{
    connect(windowManager, &WindowManager::exitThread, this,
            &GstPlayer::stopThread);
}

void GstPlayer::run()
{
    GstBus* bus;
    GError* err = nullptr;

    pipelineStr = url.toUtf8().constData();
    pipelineStr += " name=sink";
    pipeline = gst_parse_launch(pipelineStr.c_str(), &err);
    if (pipeline == nullptr) {
        qWarning() << "Error: Pipeline could not be created.";
        return;
    }

    sink = (GstVideoOverlay*)gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (sink == nullptr) {
        qWarning() << "Error: Sink could not be created.";
        return;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    if (bus != nullptr) {
        gst_bus_enable_sync_message_emission(bus);
        gst_bus_set_sync_handler(bus, gstBusCallback, gpointer(this), nullptr);
        gst_object_unref(bus);
    } else {
        qWarning() << "Error: Could not get the bus.";
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    exec();

    gst_element_set_state (pipeline, GST_STATE_READY);
    gst_element_set_state (pipeline, GST_STATE_NULL);

    if (sink != nullptr) {
        gst_object_unref (sink);
        sink = nullptr;
    }
    if (pipeline != nullptr) {
        gst_object_unref (pipeline);
        pipeline = nullptr;
    }
    windowManager->threadFinished();
}

void GstPlayer::stopThread()
{
    exit();
    wait();
}
