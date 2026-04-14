// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QProcess>
#include <cstdlib>

#include "playbackcontrol.h"
#include "videoprober.h"
#include "processhelper.h"

using namespace std;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");

    string machine;
    QProcess process;
    process.start("uname", QStringList() << "-n");
    process.waitForFinished(-1);
    if (process.exitCode() == 0) {
        QString hwrevision = process.readAllStandardOutput().trimmed();
        QStringList parts = hwrevision.split(" ", Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            QString machName = parts.first();
            machine = machName.toStdString();
        }
    }

    PlaybackControl playbackControl(machine);
    SettingsManager settingsManager;
    VideoProber videoProber;
    ProcessHelper processHelper;

    QString qmlStr;
    if (playbackControl.machine == SL1620_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1620-ai.qml";
    } else if (playbackControl.machine == SL1640_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1640-ai.qml";
    } else if (playbackControl.machine == SL1680_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1680-ai.qml";
        system(R"(echo "mode=1" > /proc/vsi/isp_subdev0)");
        system(R"(echo "sensor=ov5647 xml=/usr/share/OV5647_2k.xml manu_json=/usr/share/ISP_Manual_OV5647_2k.json auto_json=/usr/share/ISP_Auto_OV5647_2k.json" > /proc/vsi/isp_subdev0)");
        system(R"(echo "csi_clock_rate=300000000" > /proc/vsi/isp_subdev0)");
    }

    if (qmlStr.isEmpty()) {
        return 1;
    }
    const QUrl url = QUrl(qmlStr);
    QQmlApplicationEngine engine;

#ifdef HAVE_X11
    engine.rootContext()->setContextProperty("isX11", true);
#else
    engine.rootContext()->setContextProperty("isX11", false);
#endif

    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);
    engine.rootContext()->setContextProperty("settingsManager", &settingsManager);
    engine.rootContext()->setContextProperty("videoProber", &videoProber);
    engine.rootContext()->setContextProperty("processHelper", &processHelper);

    qmlRegisterSingletonType(QStringLiteral("qrc:/qmls/res.qml"), "Resource", 1, 0, "Resource");

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
