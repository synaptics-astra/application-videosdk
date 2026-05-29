// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QProcess>
#include <cstdlib>

#include "playbackcontrol.h"

using namespace std;

static void displayUsage()
{
   qDebug() << "\nUsage: syna-video-player [options]\n"
           "\n"
           "Example: syna-video-player --mode ffmpeg\n"
           "\n"
           "Options:\n"
           "         --mode (-m)    Whether to go for ffmpeg or HW decoding\n"
           "                          [ffmpeg/v4l2]\n"
           "\n"
           "Note: Push the video files to /home/root/demos/videos/ directory";
}

static void setDefaultMode(PlaybackControl &playbackControl)
{
    if (playbackControl.machine == SL1620_ASTRA_MACHINE) {
        playbackControl.mode = USE_SOFTWARE_DECODER;
    } else if (playbackControl.machine == SL2619_ASTRA_MACHINE) {
        playbackControl.mode = USE_SOFTWARE_DECODER;
    } else if (playbackControl.machine == SL2615_ASTRA_MACHINE) {
        playbackControl.mode = USE_SOFTWARE_DECODER;
    } else if (playbackControl.machine == SL1640_ASTRA_MACHINE) {
        playbackControl.mode = USE_HARDWARE_DECODER;
    } else if (playbackControl.machine == SL1680_ASTRA_MACHINE) {
        playbackControl.mode = USE_HARDWARE_DECODER;
        system(R"(echo "mode=1" > /proc/vsi/isp_subdev0)");
        system(R"(echo "sensor=ov5647 xml=/usr/share/OV5647_2k.xml manu_json=/usr/share/ISP_Manual_OV5647_2k.json auto_json=/usr/share/ISP_Auto_OV5647_2k.json" > /proc/vsi/isp_subdev0)");
        system(R"(echo "csi_clock_rate=300000000" > /proc/vsi/isp_subdev0)");
    }
}

static void handleCommandLineOptions(QStringList args, string &mode)
{
    bool modeSpecified = false;
    if (args.count() > 1) {
        for (int i = 1; i < args.count(); i++) {
            QString arg = args.at(i);
            if (arg == "-m" || arg == "--mode") {
                modeSpecified = true;
                if (i + 1 < args.count()) {
                    QString nextArg = args.at(i + 1);
                    if (nextArg == USE_SOFTWARE_DECODER) {
                        mode = USE_SOFTWARE_DECODER;
                    } else if (nextArg == USE_HARDWARE_DECODER) {
                        mode = USE_HARDWARE_DECODER;
                    }
                }
                i++;
            } else if (arg == "-h" || arg == "--help") {
                displayUsage();
                exit(0);
            }
        }
    }
    if (modeSpecified && mode.empty()) {
        displayUsage();
        exit(1);
    }
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");
    qputenv("QT_DISABLE_SHADER_DISK_CACHE", "1");

    QStringList args = app.arguments();

    QProcess process;
    process.start("uname", QStringList() << "-n");
    process.waitForFinished();
    string machine = process.readAllStandardOutput().trimmed().toStdString();

    string mode = "";
    PlaybackControl playbackControl(machine, mode);
    SettingsManager settingsManager;

    setDefaultMode(playbackControl);

    handleCommandLineOptions(args, mode);
    if (!mode.empty()) {
        playbackControl.mode = mode;
    }

    QString qmlStr;
    if (playbackControl.machine == SL1620_ASTRA_MACHINE) {
        qmlStr = "qrc:/qmls/sl1620.qml";
    } else if (playbackControl.machine == SL2619_ASTRA_MACHINE) {
        qmlStr = "qrc:/qmls/sl2619.qml";
    } else if (playbackControl.machine == SL2615_ASTRA_MACHINE) {
        qmlStr = "qrc:/qmls/sl2615.qml";
    } else if (playbackControl.machine == SL1640_ASTRA_MACHINE) {
        qmlStr = "qrc:/qmls/sl1640.qml";
    } else if (playbackControl.machine == SL1680_ASTRA_MACHINE) {
        qmlStr = "qrc:/qmls/sl1680.qml";
    }

    if (qmlStr.isEmpty()) {
        displayUsage();
        return 1;
    }
    const QUrl url = QUrl(qmlStr);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);
    engine.rootContext()->setContextProperty("settingsManager", &settingsManager);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
