// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "frameprovider.h"
#include "framecontroller.h"
#include "videoprober.h"
#include "filemanager.h"
#include "cameramanager.h"
#include "cameraimageprovider.h"
#include "filedialoghelper.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setOrganizationName("Synaptics");
    app.setOrganizationDomain("synaptics.com");
    app.setApplicationName("SuperResSlideshow");
    qputenv("QT_DISABLE_SHADER_DISK_CACHE", "1");

    QQmlApplicationEngine engine;

    //FrameProvider: NO QObject parent
    FrameProvider *frameProvider = new FrameProvider;
    engine.addImageProvider("frameProvider", frameProvider);

    engine.rootContext()->setContextProperty("frameProvider", frameProvider);
    //Controller owns logic & is exposed to QML
    FrameController *controller = new FrameController(frameProvider);
    engine.rootContext()->setContextProperty("frameController", controller);

    // Other unchanged objects
    VideoProber videoProber;
    FileManager fileManager;
    FileDialogHelper fileDialogHelper;
    CameraImageProvider *cameraImageProvider = new CameraImageProvider();
    CameraManager cameraManager(cameraImageProvider);

    engine.rootContext()->setContextProperty("videoProber", &videoProber);
    engine.rootContext()->setContextProperty("fileManager", &fileManager);
    engine.rootContext()->setContextProperty("fileDialogHelper", &fileDialogHelper);
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
