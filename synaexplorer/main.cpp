// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QProcess>

#include "playbackcontrol.h"
#include "settingsmanager.h"
#include "processhelper.h"
#include "audiohelper.h"

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

    QQmlApplicationEngine engine;

#ifdef HAVE_X11
    engine.rootContext()->setContextProperty("isX11", true);
#endif

#ifdef HAVE_WAYLAND
    engine.rootContext()->setContextProperty("isX11", false);
#endif

#ifdef HAS_NPU
    engine.rootContext()->setContextProperty("hasNPU", true);
#else
    engine.rootContext()->setContextProperty("hasNPU", false);
#endif

    engine.rootContext()->setContextProperty("playbackControl", &playbackControl);

    SettingsManager settingsManager;
    engine.rootContext()->setContextProperty("settingsManager", &settingsManager);

    ProcessHelper processHelper;
    engine.rootContext()->setContextProperty("processHelper", &processHelper);

    AudioHelper audioHelper;
    engine.rootContext()->setContextProperty("AudioHelper", &audioHelper);

    qmlRegisterSingletonType(QStringLiteral("qrc:/qmls/res.qml"), "Resource", 1, 0, "Resource");
    qmlRegisterSingletonType(QStringLiteral("qrc:/qmls/pageconfig.qml"), "PageConfig", 1, 0, "PageConfig");
    const QUrl url(QStringLiteral("qrc:/qmls/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
            &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
            }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
