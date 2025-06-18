// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "settingscontrol.h"
#include "DisplaySettings.h"
#include "wifiapp.h"
#include "bluetoothapp.h"

using namespace std;
string machine;

#define SL1620_ASTRA_MACHINE "sl1620"
#define SL1640_ASTRA_MACHINE "sl1640"
#define SL1680_ASTRA_MACHINE "sl1680"

static void displayUsage() {
    qDebug() << "\nUsage: syna-settings [options]\n"
        "\n"
        "Example: syna-settings --mach sl1680\n"
        "\n"
        "Options:\n"
        "         --mach (-M)    Your machine name [sl1620/sl1640/sl1680]\n"
        "\n";
}

static void handleCommandLineOptions(QStringList args, string &mach) {
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

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");

    QStringList args = app.arguments();
    handleCommandLineOptions(args, machine);

    if (machine.empty()) {
        displayUsage();
        return 1;
    }

    QQmlApplicationEngine engine;
    SettingsControl settingsControl;
    engine.rootContext()->setContextProperty("settingsControl", &settingsControl);

    Wifiapp wifiApp;
    engine.rootContext()->setContextProperty("wifiapp", &wifiApp);

    Bluetoothapp bluetoothApp;
    engine.rootContext()->setContextProperty("bluetoothapp", &bluetoothApp);

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
