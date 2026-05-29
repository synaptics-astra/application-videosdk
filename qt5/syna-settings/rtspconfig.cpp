// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <glib.h>
#include <gst/rtsp/gstrtsp.h>
#include <gst/sdp/gstsdpmessage.h>
#include "rtspconfig.h"
#include <QFile>

RtspConfig::RtspConfig(QObject *parent)
    : QObject(parent) {
}
const gchar *get_format_string(const gchar *media_type, const gchar *format) {
    if (g_strcmp0(media_type, "video") == 0) {
        if (g_strcmp0(format, "96") == 0) {
            return "H.264";
        } else {
            return format;
        }
    } else if (g_strcmp0(media_type, "audio") == 0) {
        if (g_strcmp0(format, "8") == 0) {
            return "G.711 µ-law";
        } else if (g_strcmp0(format, "0") == 0) {
            return "PCMA (G.711)";
        } else if (g_strcmp0(format, "18") == 0) {
            return "G.729";
        } else {
            return format;
        }
    }
    return format;
}

GstRTSPResult check_camera_live(const gchar *url_str, gint64 timeout, QVariantMap &resultMap) {

    GstRTSPUrl *url = NULL;
    GstRTSPConnection *conn = NULL;
    GstRTSPResult result;

    result = gst_rtsp_url_parse(url_str, &url);
    if (result != GST_RTSP_OK) {
        resultMap["status"] = "Error";
        resultMap["message"] = "Failed to parse RTSP URL";
        return result;
    }

    result = gst_rtsp_connection_create(url, &conn);
    if (result != GST_RTSP_OK) {
        resultMap["status"] = "Error";
        resultMap["message"] = "Failed to create connection";
        gst_rtsp_url_free(url);
        return result;
    }

    result = gst_rtsp_connection_connect_usec(conn, timeout);
    if (result != GST_RTSP_OK) {
        resultMap["status"] = "Error";
        resultMap["message"] = "RTSP camera not live";
        gst_rtsp_connection_close(conn);
        gst_rtsp_connection_free(conn);
        gst_rtsp_url_free(url);
        return result;
    }

    GstRTSPMessage *request = NULL;
    GstRTSPMessage *response = NULL;

    result = gst_rtsp_message_new_request(&request, GST_RTSP_DESCRIBE, url_str);
    if (result != GST_RTSP_OK) {
        resultMap["status"] = "Error";
        resultMap["message"] = "Failed to create DESCRIBE request";
        gst_rtsp_connection_close(conn);
        gst_rtsp_connection_free(conn);
        gst_rtsp_url_free(url);
        return result;
    }

    result = gst_rtsp_connection_send_messages_usec(conn, request, 1, timeout);
    if (result != GST_RTSP_OK) {
        resultMap["status"] = "Error";
        resultMap["message"] = "Failed to send DESCRIBE request";
        gst_rtsp_message_free(request);
        gst_rtsp_connection_close(conn);
        gst_rtsp_connection_free(conn);
        gst_rtsp_url_free(url);
        return result;
    }

    result = gst_rtsp_message_new(&response);
    if (result != GST_RTSP_OK) {
        resultMap["status"] = "Error";
        resultMap["message"] = "Failed to allocate response message";
        gst_rtsp_message_free(request);
        gst_rtsp_connection_close(conn);
        gst_rtsp_connection_free(conn);
        gst_rtsp_url_free(url);
        return result;
    }

    result = gst_rtsp_connection_receive_usec(conn, response, timeout);
    if (result == GST_RTSP_OK) {
        guint8 *body_data = NULL;
        guint body_size = 0;

        if (gst_rtsp_message_get_body(response, &body_data, &body_size) == GST_RTSP_OK && body_data != NULL) {
            GstSDPMessage *sdp_msg = NULL;
            if (gst_sdp_message_new(&sdp_msg) == GST_SDP_OK &&
                gst_sdp_message_parse_buffer(body_data, body_size, sdp_msg) == GST_SDP_OK) {
                guint media_count = gst_sdp_message_medias_len(sdp_msg);

                for (guint i = 0; i < media_count; i++) {
                    const GstSDPMedia *media = gst_sdp_message_get_media(sdp_msg, i);
                    if (media) {
                        const gchar *media_type = gst_sdp_media_get_media(media);
                        QVariantMap mediaInfo;
                        mediaInfo["type"] = media_type;
                        mediaInfo["format"] = get_format_string(media_type, gst_sdp_media_get_format(media, 0));
                        mediaInfo["resolution"] = gst_sdp_media_get_attribute_val(media, "x-dimensions") ?: "Unknown";
                        mediaInfo["framerate"] = gst_sdp_media_get_attribute_val(media, "x-framerate") ?: "Unknown";

                        resultMap[QString("stream_%1").arg(i)] = mediaInfo;
                    }
                }
                resultMap["status"] = "Live";
                gst_sdp_message_free(sdp_msg);
            }
        }
    }

    gst_rtsp_message_free(request);
    gst_rtsp_message_free(response);
    gst_rtsp_connection_close(conn);
    gst_rtsp_connection_free(conn);
    gst_rtsp_url_free(url);

    return result;
}

QVariantMap RtspConfig::validateRtspUrl(const QString &url) {
    QVariantMap result;
    check_camera_live(url.toUtf8().constData(), 1000000, result);
    result.insert("url", url);
    return result;
}

QVariantList RtspConfig::validateAllRtspUrls(const QStringList &urls) {
    QVariantList results;

    if (urls.isEmpty()) {
        qDebug() << "validateAllRtspUrls: No valid RTSP URLs to validate";
        return results;
    }

    for (const QString &url : urls) {
        if (!url.isEmpty()) {
            results.append(validateRtspUrl(url));
        }
    }

    return results;
}

bool RtspConfig::checkFileExists(const QString &filePath) {
    QFile file(filePath);
    return file.exists();
}
