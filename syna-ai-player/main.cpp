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
    qDebug() << "\nUsage: syna-ai-player [options]\n"
        "\n"
        "Example: syna-ai-player --mach=sl1680\n"
        "\n"
        "Options:\n"
        "         --mach (-M)    Your machine name [sl1640/sl1680]\n"
        "\n"
        "Note: Push the video files to /home/root/demos/videos/ directory";
}

static QString handleCommandLineOptions(int argc, char* argv[])
{
    int option;
    int option_index = 0;
    string mach_argument;
    QString qmlStr;

    struct option long_options[] = {
        {"mach", required_argument, nullptr, 'M'},
        {nullptr, 0, nullptr, 0}
    };

    while ((option = getopt_long(argc, argv, "M:",
                    long_options, &option_index)) != -1) {
        switch (option) {
            case 'M':
                mach_argument = optarg;
                break;
            default:
                break;
        }
    }

    if (mach_argument == "sl1640") {
        qmlStr = "/home/root/demos/qmls/sl1640-ai.qml";
    } else if (mach_argument == "sl1680") {
        qmlStr = "/home/root/demos/qmls/sl1680-ai.qml";
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

    qmlRegisterSingletonType(QStringLiteral("qrc:/qmls/res.qml"), "Resource", 1, 0, "Resource");

    QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreated,
            &app,
            [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
            },
            Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
