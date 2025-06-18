// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "command.h"

Command::Command()
{
    gst_init(NULL, NULL); // Initialize GStreamer
    qtDevice = new QtDevice();
    inputType = "";
    inputSource = "";
    upscalingType = "640x360_1920x1080";
    modelType = "FAST";
    srEnabled = true;
    width = "640";
    height = "360";
    qtDevices = qtDevice->getDevicePath();
    if (qtDevices.isEmpty()) {
        inputType = "";
        inputSource = "";
        qWarning() << "No camera devices detected.";
    } else {
        inputType = QString::fromStdString(qtDevices[0][0]);
        inputSource = QString::fromStdString(qtDevices[0][1]);
    }
}

Command::~Command()
{
    delete qtDevice;
}

bool Command::isResolutionSupported(int &videoWidth, int &videoHeight)
{
    if (inputType != "File Source" || inputSource.isEmpty() || !QFile::exists(inputSource)) {
        return true; // Non-file sources or invalid files bypass resolution check
    }

    VideoProber prober;
    QVariantMap videoInfo = prober.probeFile(inputSource);
    videoWidth = videoInfo.value("width").toInt();
    videoHeight = videoInfo.value("height").toInt();

    QString videoRes = QString("%1x%2").arg(videoWidth).arg(videoHeight);
    QStringList supportedRes = {"640x360", "960x540", "1280x720"};
    return supportedRes.contains(videoRes);
}

QString Command::createPipeline()
{

    // Set model and pipeline parameters for File Source
    if (inputType == "File Source" && !inputSource.isEmpty() && QFile::exists(inputSource)) {
        VideoProber prober;
        QVariantMap videoInfo = prober.probeFile(inputSource);
        int videoWidth = videoInfo.value("width").toInt();
        int videoHeight = videoInfo.value("height").toInt();

        QString videoRes = QString("%1x%2").arg(videoWidth).arg(videoHeight);
        if (videoRes == "640x360") {
            upscalingType = "640x360_1920x1080";
            width = "640";
            height = "360";
        } else if (videoRes == "960x540") {
            upscalingType = "960x540_3840x2160";
            width = "960";
            height = "540";
        } else if (videoRes == "1280x720") {
            upscalingType = "1280x720_3840x2160";
            width = "1280";
            height = "720";
        } else {
            qWarning() << "Unsupported resolution: " << videoRes;
            pipeline = "videotestsrc ! waylandsink name=sink";
            return pipeline;
        }
    }

    if (inputType == "File Source" && upscalingType.isEmpty()) {
        qWarning() << "No valid upscaling type determined for File Source";
        pipeline = "videotestsrc ! waylandsink name=sink";
        return pipeline;
    }

    QString modelPath;
    if (upscalingType == "640x360_1920x1080") {
        modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_qdeo_y_uv_640x360_1920x1080/model.synap";
        width = "640";
        height = "360";
    }
    else if (upscalingType == "960x540_3840x2160") {
        if (modelType == "FAST")
            modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_fast_y_uv_960x540_3840x2160/model.synap";
        else
            modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_qdeo_y_uv_960x540_3840x2160/model.synap";
        width = "960";
        height = "540";
    }
    else if (upscalingType == "1280x720_3840x2160") {
        if (modelType == "FAST")
            modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_fast_y_uv_1280x720_3840x2160/model.synap";
        else
            modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_qdeo_y_uv_1280x720_3840x2160/model.synap";
        width = "1280";
        height = "720";
    }
    else if (upscalingType == "1920x1080_3840x2160") {
        if (modelType == "FAST")
            modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_fast_y_uv_1920x1080_3840x2160/model.synap";
        else
            modelPath = "/usr/share/synap/models/image_processing/super_resolution/model/sr_qdeo_y_uv_1920x1080_3840x2160/model.synap";
        width = "1920";
        height = "1080";
    }
    else {
        qWarning() << "Invalid upscaling type: " << upscalingType;
        pipeline = "videotestsrc ! waylandsink name=sink";
        return pipeline;
    }

    // Validate pipeline components
    if (inputType == "File Source") {
        if (inputSource.isEmpty() || !QFile::exists(inputSource)) {
            pipeline = "videotestsrc ! waylandsink name=sink";
        } else {
            QString escapedSource = QString(inputSource).replace(" ", "\\ ");
            if (srEnabled) {
                pipeline = "filesrc location=" + escapedSource + " ! qtdemux name=demux demux.video_0 ! h264parse ! v4l2h264dec ! synapimageproc model=" + modelPath + " ! waylandsink name=sink";
            } else {
                qDebug() << "Super resolution is disabled";
                pipeline = "filesrc location=" + escapedSource + " ! qtdemux name=demux demux.video_0 ! h264parse ! v4l2h264dec ! waylandsink name=sink";
            }
        }
    } else {
        v4l2src = inputSource;
        if (v4l2src.isEmpty()) {
            qWarning() << "No camera source found!";
            pipeline = "videotestsrc ! waylandsink name=sink";
        } else {
            if (srEnabled) {
                pipeline = "v4l2src device=" + v4l2src + " ! video/x-raw,framerate=30/1,format=NV12,width=" + width + ",height=" + height + " ! synapimageproc model=" + modelPath + " ! waylandsink name=sink";
            } else {
                qDebug() << "Super resolution is disabled";
                pipeline = "v4l2src device=" + v4l2src + " ! video/x-raw,framerate=30/1,format=NV12,width=" + width + ",height=" + height + " ! waylandsink name=sink";
            }
        }
    }

    // Test pipeline validity
    GError *error = NULL;
    GstElement *testPipeline = gst_parse_launch(pipeline.toUtf8().constData(), &error);
    if (!testPipeline || error) {
        qCritical() << "Failed to create pipeline: " << (error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        if (testPipeline) gst_object_unref(testPipeline);
        pipeline = "videotestsrc ! waylandsink name=sink";
    } else {
        gst_object_unref(testPipeline);
    }

    return pipeline;
}

QString Command::getCamNode(QString inputSource)
{
    if (inputSource == "File Source") {
        return inputSource;
    }
    for (const auto& row : qtDevices) {
        if (row.size() >= 2 && QString::fromStdString(row[0]) == inputSource) {
            return QString::fromStdString(row[1]);
        }
    }
    return QString();
}

void Command::setInputType(QString inputType)
{
    this->inputType = inputType;
}

void Command::setInputSource(QString inputSource)
{
    this->inputSource = inputSource;
}

void Command::setUpscalingType(QString upscalingType)
{
    if (inputType == "File Source") {
        this->upscalingType = "";
    } else if (upscalingType == "1920x1080_3840x2160") {
        qWarning() << "Invalid upscaling type for non-File Source: " << upscalingType << ". Defaulting to 640x360_1920x1080.";
        this->upscalingType = "640x360_1920x1080";
    } else {
        this->upscalingType = upscalingType;
    }
}

void Command::setModelType(QString modelType)
{
    this->modelType = modelType;
}

void Command::srEnable(bool enable)
{
    this->srEnabled = enable;
}
