// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include <getopt.h>
#include <string>

#include "playbackcontrol.h"

using namespace std;

static void displayUsage()
{
   qDebug() << "\nUsage: syna-video-player [options]\n"
           "\n"
           "Example: syna-video-player --mach=sl1680 --mode=ffmpeg\n"
           "\n"
           "Options:\n"
           "         --mode (-m)    Whether to go for ffmpeg or HW decoding\n"
           "                          [ffmpeg/v4l2]\n"
           "         --mach (-M)    Your machine name [sl1620/sl1640/sl1680]\n"
           "\n"
           "Note: Push the video files to /home/root/demos/videos/ directory";
}

static QString handleCommandLineOptions(int argc, char* argv[])
{
    int option;
    int option_index = 0;
    string mode_argument;
    string mach_argument;
    QString qmlStr;

    struct option long_options[] = {
        {"mode", required_argument, nullptr, 'm'},
        {"mach", required_argument, nullptr, 'M'},
        {nullptr, 0, nullptr, 0}
    };

    while ((option = getopt_long(argc, argv, "m:M:",
                    long_options, &option_index)) != -1) {
        switch (option) {
            case 'm':
                mode_argument = optarg;
                break;
            case 'M':
                mach_argument = optarg;
                break;
            default:
                break;
        }
    }

    if (mach_argument == "sl1620") {
        if (mode_argument == "ffmpeg") {
            qmlStr = "/home/root/demos/qmls/sl1620.qml";
        }
    } else if (mach_argument == "sl1640") {
        if (mode_argument == "ffmpeg") {
            qmlStr = "/home/root/demos/qmls/sl1640-ffmpeg.qml";
        } else if (mode_argument == "v4l2") {
            qmlStr = "/home/root/demos/qmls/sl1640-v4l2.qml";
        }
    } else if (mach_argument == "sl1680") {
        if (mode_argument == "ffmpeg") {
            qmlStr = "/home/root/demos/qmls/sl1680-ffmpeg.qml";
        } else if (mode_argument == "v4l2") {
            qmlStr = "/home/root/demos/qmls/sl1680-v4l2.qml";
        }
    }
    return qmlStr;
}

int main(int argc, char *argv[])
{
    QString qmlStr = handleCommandLineOptions(argc, argv);
    if (qmlStr.isEmpty()) {
        displayUsage();
        return 1;
    }
    const QUrl url = QUrl(qmlStr);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");

    PlaybackControl playbackControl;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
