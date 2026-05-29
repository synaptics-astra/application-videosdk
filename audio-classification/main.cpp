// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "appsink_qml.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    qputenv("QT_DISABLE_SHADER_DISK_CACHE", "1");
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qmls/main.qml"));

    qmlRegisterType<AppSink>("com.synaptics.AppSink", 1, 0, "AppSink");
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
