// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "gstplayer.h"
#include <QScreen>

GstPlayer::GstPlayer(QWidget* parent) : QWidget(parent), pipeline(nullptr), sink(nullptr), surface(nullptr), videoInfoTimer(nullptr), lastWidth(-1), lastHeight(-1), lastFps(-1.0)
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
    gst_init(NULL, NULL);

    videoInfoTimer = new QTimer(this);
    connect(videoInfoTimer, &QTimer::timeout, this, &GstPlayer::updateVideoInfo);
}

GstPlayer::~GstPlayer()
{
    cleanupPipeline();
    delete command;
    delete videoInfoTimer;
}

bool GstPlayer::configurePlatformSpecificOverlay()
{
    if (!sink || !GST_IS_VIDEO_OVERLAY(sink)) {
        qWarning() << "Invalid sink for platform-specific overlay configuration";
        return false;
    }

    int renderWidth = width();
    int renderHeight = height();
    QString machine = command->machine;
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];

#if defined(HAVE_X11)
    WId xwinid = winId();
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), xwinid);
#elif defined(HAVE_WAYLAND)
    if (surface) {
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), (guintptr)surface);
    } else {
        qWarning() << "Wayland surface not initialized for screen:" << targetScreen->name();
        return false;
    }
#else
    qWarning() << "No supported platform (X11 or Wayland) defined";
    return false;
#endif

    // Set rendering rectangle to the widget's geometry
    gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(sink), x(), y(), renderWidth, renderHeight);
    return true;
}

void GstPlayer::startPlayback()
{
    cleanupPipeline();

    GError* error = NULL;
    QString url = command->createPipeline(width(), height());
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

    if (!configurePlatformSpecificOverlay()) {
        qWarning() << "Failed to configure platform-specific overlay";
        cleanupPipeline();
        return;
    }

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to set pipeline to PLAYING state";
        cleanupPipeline();
    } else {
        videoInfoTimer->start(1000);
    }
}

void GstPlayer::stopPlayback()
{
    videoInfoTimer->stop();
    lastWidth = -1;
    lastHeight = -1;
    lastFps = -1.0;
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
        QString machine = command->machine;
        QList<QScreen*> screens = QGuiApplication::screens();
        QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];
        int renderWidth = width();
        int renderHeight = height();
        gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(sink), x(), y(), renderWidth, renderHeight);
    }
}

void GstPlayer::updateVideoInfo()
{
    if (!pipeline || !sink) {
        return;
    }

    GstPad* sinkPad = gst_element_get_static_pad(GST_ELEMENT(sink), "sink");
    if (!sinkPad) {
        return;
    }

    GstCaps* caps = gst_pad_get_current_caps(sinkPad);
    if (caps) {
        GstStructure* structure = gst_caps_get_structure(caps, 0);
        if (structure) {
            gint width, height;
            gint fps_n, fps_d;

            bool isResolutionChanged = false;
            QString prevResolution = (lastWidth >= 0 && lastHeight >= 0) ? QString("%1x%2").arg(lastWidth).arg(lastHeight) : "N/A";
            if (gst_structure_get_int(structure, "width", &width) &&
                gst_structure_get_int(structure, "height", &height)) {
                if (width != lastWidth || height != lastHeight) {
                    QString currResolution = QString("%1x%2").arg(width).arg(height);
                    lastWidth = width;
                    lastHeight = height;
                    isResolutionChanged = true;
                    emit resolutionChanged(prevResolution, currResolution);
                }
            }

            bool fpsChanged = false;
            if (gst_structure_get_fraction(structure, "framerate", &fps_n, &fps_d)) {
                if (fps_d != 0) {
                    float fps = (float)fps_n / fps_d;
                    if (fps != lastFps) {
                        lastFps = fps;
                        fpsChanged = true;
                    }
                }
            }

            if (isResolutionChanged || fpsChanged) {
                QString resolution = QString("%1x%2").arg(lastWidth).arg(lastHeight);
                emit videoInfoChanged(resolution, lastFps);
            }
        }
        gst_caps_unref(caps);
    }

    gst_object_unref(sinkPad);
}

GstBusSyncReply GstPlayer::gstBusCallback(GstBus* bus, GstMessage* message, gpointer user_data)
{
    GstPlayer* self = static_cast<GstPlayer*>(user_data);
    QString machine = self->command->machine;
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];

#if defined(HAVE_WAYLAND)
    if (gst_is_wl_display_handle_need_context_message(message)) {
        QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
        struct wl_display* display_handle = (struct wl_display*)native->nativeResourceForWindow("display", self->windowHandle());
        if (display_handle) {
            GstContext* context = gst_wl_display_handle_context_new(display_handle);
            gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)), context);
            gst_context_unref(context);
        } else {
            qWarning() << "Failed to get Wayland display handle for screen:" << targetScreen->name();
            display_handle = (struct wl_display*)native->nativeResourceForScreen("display", screens[0]);
            if (display_handle) {
                GstContext* context = gst_wl_display_handle_context_new(display_handle);
                gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)), context);
                gst_context_unref(context);
            } else {
                qCritical() << "Failed to get Wayland display handle even for primary screen";
            }
        }
        goto drop;
    }
#endif

    if (gst_is_video_overlay_prepare_window_handle_message(message)) {
        if (!self->configurePlatformSpecificOverlay()) {
            qWarning() << "Failed to configure video sink for screen:" << targetScreen->name();
        }
        goto drop;
    }

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_EOS:
            break;
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(message, &err, &debug);
            qCritical() << "GStreamer error:" << err->message << ", Debug:" << debug;
            g_error_free(err);
            g_free(debug);
            break;
        }
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