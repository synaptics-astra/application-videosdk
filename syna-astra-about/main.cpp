// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "about.h"
#include "launcher.h"

using namespace std;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("synaptics");
    app.setOrganizationDomain("synaptics.com");

    QQmlApplicationEngine engine;

    AboutManager aboutManager;
    engine.rootContext()->setContextProperty("aboutManager", &aboutManager);

    ChromiumLauncher chromiumLauncher;
    engine.rootContext()->setContextProperty("ChromiumLauncher", &chromiumLauncher);

    SynaexplorerLauncher synaexplorerLauncher;
    engine.rootContext()->setContextProperty("SynaexplorerLauncher",&synaexplorerLauncher);

    TerminalLauncher terminalLauncher;
    engine.rootContext()->setContextProperty("TerminalLauncher",&terminalLauncher);

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
