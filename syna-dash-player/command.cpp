// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "command.h"
#include <QScreen>
#include <QGuiApplication>

Command::Command()
{
    gst_init(NULL, NULL);
    machine = getMachine();
}

Command::~Command()
{
}

QString Command::getMachine() {
    QProcess process;
    process.start("uname -n");
    if (!process.waitForFinished() || process.exitCode() != 0)
        return "Unknown Machine";

    QString machine = process.readAllStandardOutput().trimmed();
    return machine.isEmpty() ? "Unknown Machine" : machine;
}

bool Command::isResolutionSupported(int &videoWidth, int &videoHeight)
{
    if (inputSource.isEmpty() || (inputType == "File Source" && !QFile::exists(inputSource))) {
        return false;
    }

    if (inputType == "URL") {
        videoWidth = 1280;
        videoHeight = 720;
        return true;
    }

    VideoProber prober;
    QVariantMap videoInfo = prober.probeFile(inputSource);
    videoWidth = videoInfo.value("width").toInt();
    videoHeight = videoInfo.value("height").toInt();

    QString videoRes = QString("%1x%2").arg(videoWidth).arg(videoHeight);
    QStringList supportedRes = {"640x360", "960x540", "1280x720"};
    return supportedRes.contains(videoRes);
}

QString Command::createPipeline(int widgetWidth, int widgetHeight)
{
    QString detectedMachine = getMachine();
    QString sink;
    QString scale;

    // Check if video resolution exceeds widget size
    int videoWidth = 0, videoHeight = 0;
    bool isSupported = isResolutionSupported(videoWidth, videoHeight);
    if (isSupported && (videoWidth > widgetWidth || videoHeight > widgetHeight)) {
        scale = QString("videoscale ! video/x-raw,width=%1,height=%2 ! ").arg(widgetWidth).arg(widgetHeight);
    } else {
        scale = "";
    }

#ifdef HAVE_WAYLAND
    sink = "waylandsink name=sink";
#endif
#ifdef HAVE_X11
    sink = "videoconvert ! video/x-raw ! ximagesink name=sink";
#endif

    if (inputSource.isEmpty()) {
        pipeline = QString("videotestsrc ! video/x-raw,width=1280,height=720 ! textoverlay text='No input source detected..!' ! %1").arg(sink);
    } else if (inputType == "URL") {
        QString escapedSource = QString(inputSource).replace(" ", "\\ ");
        if (detectedMachine == "sl1620")
        {
            pipeline = QString("souphttpsrc location=%1 ! queue ! dashdemux ! queue ! qtdemux ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=NV12 ! %2")
                         .arg(escapedSource, sink);
        }
        else
        {
            pipeline = QString("souphttpsrc location=%1 ! queue ! dashdemux ! queue ! qtdemux ! h264parse ! v4l2h264dec ! %2")
                         .arg(escapedSource, sink);
        }
    } else {
        QString escapedSource = QString(inputSource).replace(" ", "\\ ");
        if (detectedMachine == "sl1620")
        {
            pipeline = QString("multifilesrc location=%1 loop=true ! h264parse ! avdec_h264 ! videoconvert ! %2").arg(escapedSource, sink);
        }
        else
        {
            pipeline = QString("multifilesrc location=%1 loop=true ! h264parse ! v4l2h264dec ! videoconvert ! %2").arg(escapedSource, sink);
        }
    }

    GError *error = NULL;
    GstElement *testPipeline = gst_parse_launch(pipeline.toUtf8().constData(), &error);
    if (!testPipeline || error) {
        qCritical() << "Failed to create pipeline: " << (error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        if (testPipeline) gst_object_unref(testPipeline);
        pipeline = QString("videotestsrc ! video/x-raw,width=1280,height=720 ! textoverlay text='Unknown error detected..!' ! %1").arg(sink);
    } else {
        gst_object_unref(testPipeline);
    }

    return pipeline;
}

void Command::setInputType(QString inputType)
{
    this->inputType = inputType;
}

void Command::setInputSource(QString inputSource)
{
    this->inputSource = inputSource;
}