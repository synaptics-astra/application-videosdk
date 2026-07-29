// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QProcess>
#include "settingscontrol.h"
#include "DisplaySettings.h"
#include "wifiapp.h"
#include "bluetoothapp.h"
#include "rtspconfig.h"

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");
    QString machine = "unknown";
    qputenv("QT_DISABLE_SHADER_DISK_CACHE", "1");

    QProcess process;
    process.start("uname", QStringList() << "-n");
    process.waitForFinished(-1);
    if (process.exitCode() == 0) {
        machine = process.readAllStandardOutput().trimmed();
    }
    if (machine == "sl2619-coralboard") {
        machine = "sl2619";
    }

    QQmlApplicationEngine engine;
    SettingsControl settingsControl;
    engine.rootContext()->setContextProperty("settingsControl", &settingsControl);

    Wifiapp wifiApp;
    engine.rootContext()->setContextProperty("wifiapp", &wifiApp);

    Bluetoothapp bluetoothApp;
    engine.rootContext()->setContextProperty("bluetoothapp", &bluetoothApp);

    RtspConfig rtspConfig;
    engine.rootContext()->setContextProperty("cppRtspConfig", &rtspConfig);

    engine.rootContext()->setContextProperty("machine", machine);

    bool isSampleModuleEnabled = false;
#ifdef SAMPLE_MODULE
    isSampleModuleEnabled = true;
#endif
    engine.rootContext()->setContextProperty("isSampleModuleEnabled", isSampleModuleEnabled);
    DisplaySettings displaySettings;
    engine.rootContext()->setContextProperty("displaySettings", &displaySettings);
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
