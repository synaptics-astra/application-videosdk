// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "cameramanager.h"
#include "cameraimageprovider.h"
#include <QFile>
#include <QDir>

CameraManager::CameraManager(CameraImageProvider *imageProvider, QObject *parent)
    : QObject(parent), previewPipeline(nullptr), appSink(nullptr), pipelineBus(nullptr),
      previewImageProvider(imageProvider), isCapturing(false)
{
    gst_init(nullptr, nullptr);
    devicePath = getCameraPath();
    if (devicePath.isEmpty()) {
        qWarning() << "No USB camera detected";
    }
    QString captureDir = "/home/root/demos/captured-images";
    QDir dir(captureDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "Failed to create directory:" << captureDir;
        }
    }
}

CameraManager::~CameraManager()
{
    stopCamera();
}

QString CameraManager::getCameraPath()
{
    QProcess process;
    process.start("v4l2-ctl", QStringList() << "--list-devices");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);

    bool isUsbCamera = false;
    for (const QString &line : lines) {
        if (line.contains("usb", Qt::CaseInsensitive) && line.contains(":")) {
            isUsbCamera = true;
            continue;
        }
        if (isUsbCamera && line.contains("/dev/video")) {
            QString device = line.trimmed();
            QFile file(device);
            if (file.exists()) {
                return device;
            }
        }
        if (line.trimmed().isEmpty() || (!line.contains("/dev/") && line.contains(":"))) {
            isUsbCamera = false;
        }
    }
    return QString();
}

void CameraManager::launchCamera()
{
    if (devicePath.isEmpty() || !QFile::exists(devicePath)) {
        emit noCameraDetected("No camera detected. Please connect a camera.");
        return;
    }
    if (previewPipeline) {
        stopCamera();
    }
    setupPreviewPipeline();
}

void CameraManager::stopCamera()
{
    if (previewPipeline) {
        gst_element_set_state(previewPipeline, GST_STATE_NULL);
        if (pipelineBus) {
            gst_object_unref(pipelineBus);
            pipelineBus = nullptr;
        }
        gst_object_unref(previewPipeline);
        previewPipeline = nullptr;
    }
    if (appSink) {
        gst_object_unref(appSink);
        appSink = nullptr;
    }
    if (previewImageProvider) {
        QImage blackImage(640, 480, QImage::Format_RGB888);
        blackImage.fill(Qt::black);
        previewImageProvider->updateImage(blackImage);
        emit previewUpdated();
    }
}

void CameraManager::captureImage()
{
    if (isCapturing) {
        emit captureResult(false, "Capture in progress, please wait.");
        return;
    }
    if (!previewPipeline) {
        emit captureResult(false, "Camera is not active.");
        return;
    }

    isCapturing = true;

    // Use a fixed filename for the captured image
    QString filename = "/home/root/demos/captured-images/captured-image.png";

    // Delete any existing image at the target path
    if (QFile::exists(filename)) {
        if (!QFile::remove(filename)) {
            qCritical() << "Failed to delete existing image:" << filename;
            isCapturing = false;
            emit captureResult(false, "Failed to delete existing image.");
            return;
        }
    }

    // Stop preview pipeline
    GstStateChangeReturn nullRet = gst_element_set_state(previewPipeline, GST_STATE_NULL);
    if (nullRet == GST_STATE_CHANGE_FAILURE) {
        isCapturing = false;
        emit captureResult(false, "Failed to stop preview pipeline for capture.");
        return;
    }

    // Create capture pipeline
    QString capturePipelineStr = QString(
        "v4l2src device=%1 num-buffers=1 ! synavideoconvertscale ! "
        "video/x-raw,width=1280,height=720 ! pngenc ! filesink location=%2")
        .arg(devicePath).arg(filename);

    GError *error = nullptr;
    GstElement *capturePipeline = gst_parse_launch(capturePipelineStr.toUtf8().constData(), &error);
    if (!capturePipeline) {
        qCritical() << "Failed to create capture pipeline:" << (error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        isCapturing = false;
        setupPreviewPipeline();
        emit captureResult(false, "Failed to create capture pipeline.");
        return;
    }

    GstBus *captureBus = gst_element_get_bus(capturePipeline);
    if (!captureBus) {
        qCritical() << "Failed to get capture pipeline bus";
        gst_element_set_state(capturePipeline, GST_STATE_NULL);
        gst_object_unref(capturePipeline);
        isCapturing = false;
        setupPreviewPipeline();
        emit captureResult(false, "Failed to get capture pipeline bus.");
        return;
    }

    // Start capture pipeline
    GstStateChangeReturn playRet = gst_element_set_state(capturePipeline, GST_STATE_PLAYING);
    if (playRet == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to start capture pipeline";
        gst_element_set_state(capturePipeline, GST_STATE_NULL);
        gst_object_unref(captureBus);
        gst_object_unref(capturePipeline);
        isCapturing = false;
        setupPreviewPipeline();
        emit captureResult(false, "Failed to start capture pipeline.");
        return;
    }

    // Wait for EOS or error
    GstMessage *msg = gst_bus_timed_pop_filtered(captureBus, GST_SECOND * 5, GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
    bool success = true;
    QString message = QString("Image captured to %1").arg(filename);

    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err = nullptr;
            gchar *debug = nullptr;
            gst_message_parse_error(msg, &err, &debug);
            success = false;
            message = QString("Capture error: %1").arg(err->message);
            qCritical() << "Capture error:" << err->message << ", Debug:" << (debug ? debug : "None");
            g_error_free(err);
            g_free(debug);
        }
        gst_message_unref(msg);
    } else {
        success = false;
        message = "Capture timed out.";
        qCritical() << "No EOS or error received within 5 seconds.";
    }

    // Clean up capture pipeline
    gst_element_set_state(capturePipeline, GST_STATE_NULL);
    gst_object_unref(captureBus);
    gst_object_unref(capturePipeline);

    // Restart preview pipeline
    setupPreviewPipeline();

    isCapturing = false;
    emit captureResult(success, message);
}

void CameraManager::setupPreviewPipeline()
{
    QString pipelineStr = QString(
        "v4l2src device=%1 ! video/x-raw,framerate=30/1,format=NV12,width=640,height=480 ! "
        "synavideoconvertscale ! video/x-raw,format=RGB,width=640,height=480 ! appsink name=sink")
        .arg(devicePath);

    GError *error = nullptr;
    previewPipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);
    if (!previewPipeline) {
        qCritical() << "Failed to create preview pipeline:" << (error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        emit captureResult(false, "Failed to create preview pipeline.");
        return;
    }

    pipelineBus = gst_element_get_bus(previewPipeline);
    if (!pipelineBus) {
        qCritical() << "Failed to get preview pipeline bus";
        gst_element_set_state(previewPipeline, GST_STATE_NULL);
        gst_object_unref(previewPipeline);
        previewPipeline = nullptr;
        emit captureResult(false, "Failed to get preview pipeline bus.");
        return;
    }

    appSink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(previewPipeline), "sink"));
    if (!appSink) {
        qCritical() << "Failed to get appsink element";
        gst_element_set_state(previewPipeline, GST_STATE_NULL);
        gst_object_unref(pipelineBus);
        gst_object_unref(previewPipeline);
        pipelineBus = nullptr;
        previewPipeline = nullptr;
        emit captureResult(false, "Failed to get appsink element.");
        return;
    }

    // Set up appsink callbacks
    GstAppSinkCallbacks callbacks = { nullptr, nullptr, CameraManager::newSampleCallback, nullptr };
    gst_app_sink_set_callbacks(appSink, &callbacks, this, nullptr);
    gst_app_sink_set_drop(appSink, true);
    gst_app_sink_set_max_buffers(appSink, 1);

    // Monitor bus for errors during startup
    GstBus *bus = gst_element_get_bus(previewPipeline);
    while (GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_MSECOND * 100, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_WARNING))) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err = nullptr;
            gchar *debug = nullptr;
            gst_message_parse_error(msg, &err, &debug);
            qCritical() << "Preview pipeline error during setup:" << err->message << ", Debug:" << (debug ? debug : "None");
            g_error_free(err);
            g_free(debug);
            gst_message_unref(msg);
            gst_object_unref(bus);
            stopCamera();
            emit captureResult(false, "Preview pipeline error during setup.");
            return;
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_WARNING) {
            GError *err = nullptr;
            gchar *debug = nullptr;
            gst_message_parse_warning(msg, &err, &debug);
            qWarning() << "Preview pipeline warning during setup:" << err->message << ", Debug:" << (debug ? debug : "None");
            g_error_free(err);
            g_free(debug);
        }
        gst_message_unref(msg);
    }
    gst_object_unref(bus);

    // Start preview pipeline
    GstStateChangeReturn ret = gst_element_set_state(previewPipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to start preview pipeline";
        stopCamera();
        emit captureResult(false, "Failed to start preview pipeline.");
        emit captureResult(false, "Failed to start preview pipeline.");
        return;
    }

    // Wait for state change to complete
    GstState state, pending;
    ret = gst_element_get_state(previewPipeline, &state, &pending, GST_SECOND * 5);
    if (ret == GST_STATE_CHANGE_FAILURE || state != GST_STATE_PLAYING) {
        qCritical() << "Preview pipeline failed to reach PLAYING state, current:" << gst_element_state_get_name(state)
                    << ", pending:" << gst_element_state_get_name(pending);
        stopCamera();
        emit captureResult(false, "Failed to reach PLAYING state for preview.");
        return;
    }
    emit previewStarted();
}

GstFlowReturn CameraManager::newSampleCallback(GstAppSink *sink, gpointer user_data)
{
    CameraManager *self = reinterpret_cast<CameraManager*>(user_data);
    if (!self) {
        qCritical() << "Invalid user data in newSampleCallback";
        return GST_FLOW_ERROR;
    }

    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (!sample) {
        qCritical() << "Failed to pull sample from appsink";
        return GST_FLOW_ERROR;
    }

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (!buffer) {
        qCritical() << "Failed to get buffer from sample";
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        qCritical() << "Failed to map buffer";
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    if (self->previewImageProvider) {
        QImage image(map.data, 640, 480, QImage::Format_RGB888);
        if (image.isNull()) {
            qCritical() << "Failed to create QImage from buffer, size:" << map.size;
        } else {
            self->previewImageProvider->updateImage(image.copy());
            emit self->previewUpdated();
        }
    }

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}
