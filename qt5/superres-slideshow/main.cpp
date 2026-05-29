// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "frameprovider.h"
#include "videoprober.h"
#include "filemanager.h"
#include "cameramanager.h"
#include "cameraimageprovider.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    app.setOrganizationName("Synaptics");
    app.setOrganizationDomain("synaptics.com");
    app.setApplicationName("SuperResSlideshow");

    QQmlApplicationEngine engine;

    FrameProvider *frameProvider = new FrameProvider(&app);
    VideoProber videoProber;
    FileManager fileManager;
    CameraImageProvider *cameraImageProvider = new CameraImageProvider();
    CameraManager cameraManager(cameraImageProvider);

    engine.rootContext()->setContextProperty("frameProvider", frameProvider);
    engine.addImageProvider("frameProvider", frameProvider);
    engine.rootContext()->setContextProperty("videoProber", &videoProber);
    engine.rootContext()->setContextProperty("fileManager", &fileManager);
    engine.rootContext()->setContextProperty("cameraManager", &cameraManager);
    engine.addImageProvider("cameraProvider", cameraImageProvider);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
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
