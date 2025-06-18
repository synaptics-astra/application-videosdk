// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "playbackcontrol.h"
#include "filedialoghelper.h"

using namespace std;

static void displayUsage()
{
    qDebug() << "\nUsage: syna-capability-demo [options]\n"
        "\n"
        "Example: syna-capability-demo --mach sl1680\n"
        "\n"
        "Options:\n"
        "         --mach (-M)    Your machine name [sl1620/sl1640/sl1680]\n"
        "\n"
        "Note: Push the video files to /home/root/demos/videos/ directory";
}

static void handleCommandLineOptions(QStringList args, string &mach)
{
    if (args.count() > 1) {
        for (int i = 1; i < args.count(); i++) {
            QString arg = args.at(i);
            if (arg == "-M" || arg == "--mach") {
                if (i + 1 < args.count() && args.at(i + 1) == SL1620_ASTRA_MACHINE) {
                    mach = SL1620_ASTRA_MACHINE;
                } else if (i + 1 < args.count() && args.at(i + 1) == SL1640_ASTRA_MACHINE) {
                    mach = SL1640_ASTRA_MACHINE;
                } else if (i + 1 < args.count() && args.at(i + 1) == SL1680_ASTRA_MACHINE) {
                    mach = SL1680_ASTRA_MACHINE;
                }
                i++;
            } else if (arg == "-h" || arg == "--help") {
                displayUsage();
            }
        }
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

    PlaybackControl playbackControl;
    FileControl fileControl;
    FileDialogHelper fileDialogHelper;

    QStringList args = app.arguments();
    handleCommandLineOptions(args, playbackControl.machine);

    QString qmlStr;
    if (playbackControl.machine == SL1620_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1620-capability-demo.qml";
    } else if (playbackControl.machine == SL1640_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1640-capability-demo.qml";
    } else if (playbackControl.machine == SL1680_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1680-capability-demo.qml";
    }

    if (qmlStr.isEmpty()) {
        displayUsage();
        return 1;
    }
    const QUrl url = QUrl(qmlStr);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);
    engine.rootContext()->setContextProperty("fileControl", &fileControl);
    engine.rootContext()->setContextProperty("fileDialogHelper", &fileDialogHelper);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
