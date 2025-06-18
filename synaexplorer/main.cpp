// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "playbackcontrol.h"
#include "settingsmanager.h"
#include "processhelper.h"

using namespace std;

static void displayUsage()
{
    qDebug() << "\nUsage: synaexplorer [options]\n"
        "\n"
        "Example: synaexplorer --mach sl1680\n"
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
                if (i + 1 < args.count() && args.at(i + 1) == "sl1620") {
                    mach = SL1620_ASTRA_MACHINE;
                } else if (i + 1 < args.count() && args.at(i + 1) == "sl1640") {
                    mach = SL1640_ASTRA_MACHINE;
                } else if (i + 1 < args.count() && args.at(i + 1) == "sl1680") {
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

    QStringList args = app.arguments();
    PlaybackControl playbackControl;
    handleCommandLineOptions(args, playbackControl.machine);

    if (playbackControl.machine.empty()) {
        displayUsage();
        return 1;
    }

    QQmlApplicationEngine engine;

#ifdef HAVE_X11
    engine.rootContext()->setContextProperty("isX11", true);
#endif

#ifdef HAVE_WAYLAND
    engine.rootContext()->setContextProperty("isX11", false);
#endif

    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);

    SettingsManager settingsManager;
    engine.rootContext()->setContextProperty("settingsManager", &settingsManager);

    ProcessHelper processHelper;
    engine.rootContext()->setContextProperty("processHelper", &processHelper);

    qmlRegisterSingletonType(QStringLiteral("qrc:/qmls/res.qml"), "Resource", 1, 0, "Resource");
    const QUrl url(QStringLiteral("qrc:/qmls/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
            &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
            }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
