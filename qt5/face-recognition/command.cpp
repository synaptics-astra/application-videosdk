// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "command.h"

Command::Command()
{
    qtDevice = new QtDevice();
    inputSource = "USB Camera 1";
    width = "640";
    height = "480";
    inputType = "cam";
    url = "";
    detectionModel = "/usr/share/synap/models/object_detection/face/model/yolov5s_face_640x480_onnx_mq/model.synap";
    recognitionModel = "/usr/share/synap/models/object_recognition/face/model/arcface_quant/model.synap";
    qtDevices = qtDevice->getDevicePath();
}

Command::~Command()
{
    delete qtDevice;
}

QString Command::createPipeline()
{
    v4l2src = getCamNode(inputSource);
    if (inputType == "rtsp") {
        if (urlemptyflag) {
            pipeline = "videotestsrc ! video/x-raw,framerate=30/1,format=RGB,width="+width+",height="+height+" ! textoverlay text='No url found add it first..!' !  appsink name=appSink";
        }
        else {
            pipeline = "rtspsrc location="+url+" latency=500 ! rtpjitterbuffer ! rtph264depay wait-for-keyframe=true ! video/x-h264, width=1920, height=1080 ! h264parse ! v4l2h264dec ! synavideoconvertscale ! video/x-raw,width="+width+",height="+height+" ! synapinfer mode=detector model="+detectionModel+" frameinterval=1 ! synapinfer mode=recognition model="+recognitionModel+" ! appsink name=appSink";
        }
    }
    else {
        if (v4l2src.isEmpty()) {
            qWarning() << "No cams found !!!";
            pipeline = "videotestsrc ! video/x-raw,framerate=30/1,format=RGB,width="+width+",height="+height+" ! textoverlay text='No camera source detected..!' !  appsink name=appSink";
        }
        else {
            pipeline = "v4l2src device="+v4l2src+" ! video/x-raw,framerate=30/1,format=YUY2,width="+width+",height="+height+" ! synavideoconvertscale ! video/x-raw,width="+width+",height="+height+" ! synapinfer mode=detector model="+detectionModel+" ! synapinfer mode=recognition model="+recognitionModel+" ! appsink name=appSink";
        }
    }

    return pipeline;
}

QString Command::getCamNode(QString inputSource)
{
    for (const auto& row : qtDevices) {
        if (row.size() >= 2 && QString::fromStdString(row[0]) == inputSource) {
            return QString::fromStdString(row[1]);
        }
    }
    return QString();
}
