// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "command.h"
#include <QProcess>

Command::Command()
{
    gst_init(nullptr, nullptr);
    machine = getMachine();
}

Command::~Command() = default;

QString Command::getMachine()
{
    QProcess p;
    p.start("uname", QStringList() << "-n");
    p.waitForFinished();
    return p.readAllStandardOutput().trimmed();
}

bool Command::isResolutionSupported(int&, int&) { return true; }

QString Command::createPipeline(int, int)
{
    QString pipeline;
    QString conv = "videoconvert ! video/x-raw,format=RGB ! ";
    QString src = inputSource.replace(" ", "\\ ");
    QString m = machine;

    if (inputSource.isEmpty()) {
        pipeline = "videotestsrc ! video/x-raw,width=1280,height=720,format=RGB ! "
                   "textoverlay text='No Source' ! " + conv + "appsink name=sink";
    }
    else if (inputType == "URL") {
        if (m == "sl1620") {
            pipeline = QString("souphttpsrc location=%1 ! queue ! dashdemux ! queue ! qtdemux ! "
                               "h264parse ! avdec_h264 ! %2appsink name=sink max-buffers=1 drop=true")
                       .arg(src, conv);
        } else {
            pipeline = QString("souphttpsrc location=%1 ! queue ! dashdemux ! queue ! qtdemux ! "
                               "h264parse ! v4l2h264dec ! %2appsink name=sink max-buffers=1 drop=true")
                       .arg(src, conv);
        }
    }
    else {
        if (m == "sl1620") {
            pipeline = QString("multifilesrc location=%1 loop=true ! h264parse ! avdec_h264 ! "
                               "%2appsink name=sink max-buffers=1 drop=true")
                       .arg(src, conv);
        } else {
            pipeline = QString("multifilesrc location=%1 loop=true ! h264parse ! v4l2h264dec ! "
                               "%2appsink name=sink max-buffers=1 drop=true")
                       .arg(src, conv);
        }
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
