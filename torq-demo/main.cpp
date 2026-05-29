// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QProcess>

#include "playbackcontrol.h"

using namespace std;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");

    system("v4l2-ctl -d /dev/v4l-subdev2 --set-ctrl auto_exposure=1");
    system("v4l2-ctl -d /dev/v4l-subdev2 --set-ctrl gain_automatic=0");
    system("v4l2-ctl -d /dev/v4l-subdev2 --set-ctrl white_balance_automatic=1");
    system("v4l2-ctl -d /dev/v4l-subdev2 --set-ctrl analogue_gain=80");
    system("v4l2-ctl -d /dev/v4l-subdev2 --set-ctrl exposure=850");

    string machine;
    QProcess process;
    process.start("uname", QStringList() << "-n");
    process.waitForFinished();
    machine = process.readAllStandardOutput().trimmed().toStdString();

    PlaybackControl playbackControl(machine);
    QQmlApplicationEngine engine;

#ifdef HAVE_X11
    engine.rootContext()->setContextProperty("isX11", true);
#else
    engine.rootContext()->setContextProperty("isX11", false);
#endif

    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);

    const QUrl url(QStringLiteral("qrc:/qmls/torq.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
