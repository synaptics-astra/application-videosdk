// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "gstplayer.h"
#include <QGuiApplication>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

int pth_util_fd_valid(int fd)
{
    if (fd < 3 || fd >= FD_SETSIZE)
        return FALSE;
    if (fcntl(fd, F_GETFL) == -1 && errno == EBADF)
        return FALSE;
    return TRUE;
}

int GstPlayer::list_fds()
{
     int i;
     int fd_dup;
     for (i = 0; i < FD_SETSIZE; i++) {
        is_valid_fd[i] = 0;

        if(!pth_util_fd_valid(i))
            continue;

        fd_dup = dup(i);
        if (fd_dup != -1) {
            close(fd_dup);
            is_valid_fd[i] = 1;
        }
     }
     return 0;
}

int GstPlayer::check_and_close_fds()
{
     int i;
     int fd_dup;
     int pid = getpid();
     for (i = 0; i < FD_SETSIZE; i++) {

        if(!pth_util_fd_valid(i))
            continue;
        fd_dup = dup(i);
        if (fd_dup == -1) {
            close(i);
        } else {
            close(fd_dup);

            if (is_valid_fd[i] == 0) {
                char fd_path[256];
                char target[256];

                snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%d", pid, i);
                ssize_t len = readlink(fd_path, target, sizeof(target) - 1);
                /* "/dev/video" fds are leaking cma
                    "sockets" are leaking fds without much leak of memory */
                if (len != -1 && ((strstr(target, "dev/video")))) {
                    target[len] = '\0';
                    close(i);
                }
                is_valid_fd[i] = 0;
            }
        }
     }
     return 0;
}

bool GstPlayer::configurePlatformSpecificOverlay() {
#ifdef HAVE_WAYLAND
    QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
    if (!native) return false;

    surface = static_cast<struct wl_surface*>(
        native->nativeResourceForWindow("surface", windowManager->windowHandle()));
    if (!surface) return false;

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), (guintptr)surface);
#endif

#ifdef HAVE_X11
    WId xwinid = windowManager->winId();
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), xwinid);
#endif

    gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(sink),
            x, y, width, height);
    return true;
}

GstBusSyncReply GstPlayer::gstBusCallback(GstBus* bus, GstMessage* message,
        gpointer user_data)
{
    GstPlayer* player = static_cast<GstPlayer*>(user_data);

#ifdef HAVE_WAYLAND
    if (gst_is_wl_display_handle_need_context_message(message)) {
        gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)),
                player->windowManager->context);
        goto drop;
    }
#endif

    if (gst_is_video_overlay_prepare_window_handle_message(message)) {
        if (!player->configurePlatformSpecificOverlay()) {
            qWarning() << "Failed to configure video sink";
        }
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
    connect(this, &QThread::finished, windowManager, &WindowManager::threadFinished);
}


void GstPlayer::run()
{
    GstBus* bus;
    GError* err = nullptr;
    string gstURL = url.toStdString();
    size_t index = 0;

#ifdef HAVE_WAYLAND
    string keyword("placeholdersink"), updatedKeyword("waylandsink name=sink");
#endif

#ifdef HAVE_X11
    string keyword("placeholdersink"), updatedKeyword("glimagesink name=sink");
#endif

    list_fds();

    while((index = gstURL.find(keyword, index)) != string::npos)
    {
        gstURL.replace(index, keyword.length(), updatedKeyword);
        index += updatedKeyword.length();
    }
    url = QString::fromStdString(gstURL);
    pipelineStr = url.toUtf8().constData();
    pipeline = gst_parse_launch(pipelineStr.c_str(), &err);
    if (pipeline == nullptr) {
        qWarning() << "Error: Pipeline could not be created.";
        return;
    }

    sink = (GstVideoOverlay*)gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (!sink) {
        qWarning() << "Error: Sink could not be created";
        return;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    if (bus) {
        gst_bus_enable_sync_message_emission(bus);
        gst_bus_set_sync_handler(bus, gstBusCallback, gpointer(this), nullptr);
        gst_object_unref(bus);
    } else {
        qWarning() << "Error: Could not get the bus";
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    exec();

    gst_element_set_state (pipeline, GST_STATE_READY);
    gst_element_set_state(pipeline, GST_STATE_NULL);

    if (sink) {
        gst_object_unref(sink);
        sink = nullptr;
    }
    if (pipeline) {
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
}


void GstPlayer::stopThread()
{
    quit();
    check_and_close_fds();
}
