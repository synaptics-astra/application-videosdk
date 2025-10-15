// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

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

    string machine, mode;
    QProcess process;
    process.start("uname", QStringList() << "-n");
    process.waitForFinished();
    machine = process.readAllStandardOutput().trimmed().toStdString();

    PlaybackControl playbackControl(machine, mode);
    SettingsManager settingsManager;

    QString qmlStr;
    if (playbackControl.machine == SL1620_ASTRA_MACHINE) {
        playbackControl.mode = USE_SOFTWARE_DECODER;
        qmlStr = "qrc:/qmls/sl1620.qml";
    } else if (playbackControl.machine == SL2619_ASTRA_MACHINE) {
        playbackControl.mode = USE_SOFTWARE_DECODER;
        qmlStr = "qrc:/qmls/sl2619.qml";
    } else if (playbackControl.machine == SL1640_ASTRA_MACHINE) {
        playbackControl.mode = USE_HARDWARE_DECODER;
        qmlStr = "qrc:/qmls/sl1640.qml";
    } else if (playbackControl.machine == SL1680_ASTRA_MACHINE) {
        playbackControl.mode = USE_HARDWARE_DECODER;
        qmlStr = "qrc:/qmls/sl1680.qml";
    } else {
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