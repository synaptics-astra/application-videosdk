// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QProcess>

#include "playbackcontrol.h"
#include "filedialoghelper.h"

using namespace std;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");
    qputenv("QT_DISABLE_SHADER_DISK_CACHE", "1");

    string machine;
    QString machineHostName;
    QProcess process;
    process.start("uname", QStringList() << "-n");
    process.waitForFinished();
    machineHostName = process.readAllStandardOutput().trimmed();
    machine = machineHostName.toStdString();
    if (machine == "sl2619-coralboard") {
        machine = "sl2619";
    }

    PlaybackControl playbackControl(machine);
    FileControl fileControl;
    FileDialogHelper fileDialogHelper;

    QString qmlStr;
    if (playbackControl.machine == SL1620_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1620-capability-demo.qml";
    } else if (playbackControl.machine == SL2619_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl2619-capability-demo.qml";
    } else if (playbackControl.machine == SL2615_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl2615-capability-demo.qml";
    } else if (playbackControl.machine == SL1680_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1680-capability-demo.qml";
    } else if (playbackControl.machine == SL1640_ASTRA_MACHINE) {
        qmlStr = "/home/root/demos/qmls/sl1640-capability-demo.qml";
    }

    if (qmlStr.isEmpty()) {
        return 1;
    }
    const QUrl url = QUrl(qmlStr);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);
    engine.rootContext()->setContextProperty("fileControl", &fileControl);
    engine.rootContext()->setContextProperty("fileDialogHelper", &fileDialogHelper);
    engine.rootContext()->setContextProperty("machineHostName", machineHostName);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
