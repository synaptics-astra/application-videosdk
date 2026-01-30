// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <gst/gst.h>
#include <QDebug>
#include <QStringList>
#include <QRegularExpression>
#include <QFile>
#include <QTcpSocket>

#include "webrtc_manager.h"

WebrtcManager::WebrtcManager(QObject *parent)
    : QObject(parent),
      signallingServer(nullptr),
      httpServer(nullptr),
      fileSrc(nullptr),
      aiDemoSrc(nullptr),
      cameraSrc(nullptr)
{
    gst_init(nullptr, nullptr);
}

WebrtcManager::~WebrtcManager() {
    stopServers();
    stopFileSrc();
    stopCameraSrc();
    stopAIDemos();
}

bool WebrtcManager::hasInternetConnection()
{
    QTcpSocket socket;

    socket.connectToHost("8.8.8.8", 53);
    if (socket.waitForConnected(4000)) {
        socket.close();
        return true;
    }

    socket.abort();

    socket.connectToHost("8.8.4.4", 53);
    if (socket.waitForConnected(3000)) {
        socket.close();
        return true;
    }

    return false;
}

QString WebrtcManager::getMachine() {
    static QString cachedMachine = "Unknown Machine";
    if (cachedMachine == "Unknown Machine") {
        QProcess process;
        process.start("uname -n");
        if (process.waitForFinished() && process.exitCode() == 0) {
            cachedMachine = process.readAllStandardOutput().trimmed();
            if (cachedMachine.isEmpty()) {
                cachedMachine = "Unknown Machine";
            }
        }
    }
    return cachedMachine;
}

void WebrtcManager::startServers()
{
    if (!signallingServer) {
        signallingServer = new QProcess(this);
        signallingServer->start("gst-webrtc-signalling-server");
        if (!signallingServer->waitForStarted()) {
            signallingServer->deleteLater();
            signallingServer = nullptr;
        }
    }

    if (!httpServer) {
        httpServer = new QProcess(this);
        QStringList args = { "-m", "http.server", "8000", "-d", "/home/root/demos/webrtc" };
        httpServer->start("python3", args);
        if (!httpServer->waitForStarted()) {
            httpServer->deleteLater();
            httpServer = nullptr;
        }
    }
}

void WebrtcManager::stopServers()
{
    if (signallingServer) {
        signallingServer->terminate();
        signallingServer->waitForFinished(1000);
        signallingServer->deleteLater();
        signallingServer = nullptr;
    }

    if (httpServer) {
        httpServer->terminate();
        httpServer->waitForFinished(1000);
        httpServer->deleteLater();
        httpServer = nullptr;
    }
}

QString WebrtcManager::getEthernetIP()
{
    QProcess process;
    process.start("sh", QStringList() << "-c" << "ip addr show eth0");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QRegularExpression regex(R"(inet\s+(\d+\.\d+\.\d+\.\d+))");
    QRegularExpressionMatch match = regex.match(output);

    if (match.hasMatch()) {
        return match.captured(1);
    }

    return "Unknown";
}

void WebrtcManager::stopFileSrc()
{
    if (fileSrc) {
        disconnect(fileSrc, nullptr, this, nullptr);
        fileSrc->terminate();
        fileSrc->waitForFinished(1000);
        fileSrc->deleteLater();
        fileSrc = nullptr;
    }
}

void WebrtcManager::stopAIDemos()
{
    if (aiDemoSrc) {
        disconnect(aiDemoSrc, nullptr, this, nullptr);
        aiDemoSrc->terminate();
        aiDemoSrc->waitForFinished(1000);
        aiDemoSrc->deleteLater();
        aiDemoSrc = nullptr;
    }
}

bool WebrtcManager::validateFile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }

    QString cleanFilePath = filePath;
    if (cleanFilePath.startsWith("file://")) {
        cleanFilePath = cleanFilePath.remove(0, 7);
    }

    if (!cleanFilePath.toLower().endsWith(".mp4")) {
        emit invalidFileSelected();
        return false;
    }

    QProcess discoverer;
    discoverer.start("gst-discoverer-1.0", QStringList() << ("file://" + cleanFilePath));
    if (!discoverer.waitForStarted()) {
        qWarning() << "Failed to start gst-discoverer-1.0";
        return false;
    }

    discoverer.waitForFinished();
    QString output = discoverer.readAllStandardOutput();

    bool isPlayable = false;
    bool hasVideo = false;
    QString detectedVideoCodec;

    struct SupportedCodec {
        QString extension;
        QString videoCodec;
    };

    const QList<SupportedCodec> supportedCodecs = {
        {"mp4", "h264"}
    };

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed().toLower();
        if (trimmedLine.contains("seekable: yes")) {
            isPlayable = true;
        }
        if (trimmedLine.contains("video #") && trimmedLine.contains("h.264")) {
            hasVideo = true;
            detectedVideoCodec = "h264";
        }
    }

    bool isSupported = false;
    for (const auto &codec : supportedCodecs) {
        if (cleanFilePath.toLower().endsWith("." + codec.extension) &&
            detectedVideoCodec == codec.videoCodec) {
            isSupported = true;
            break;
        }
    }

    if (!isPlayable) {
        qWarning() << "File is not playable:" << cleanFilePath;
        emit invalidFileSelected();
        return false;
    }

    if (!hasVideo || !isSupported) {
        emit invalidFileSelected();
        return false;
    }

    return true;
}

void WebrtcManager::streamFileSrc(const QString &filePath)
{
    if (!validateFile(filePath)) {
        return;
    }

    QString cleanFilePath = filePath;
    if (cleanFilePath.startsWith("file://")) {
        cleanFilePath = cleanFilePath.remove(0, 7);
    }

    stopFileSrc();

    fileSrc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GST_PLUGIN_FEATURE_RANK", "v4l2vp8enc:NONE");
    fileSrc->setProcessEnvironment(env);

    QStringList args;
    bool isMachine1620Or2619or2615 = getMachine().contains("1620") || getMachine().contains("2619") || getMachine().contains("2615");
    QString videoResolution = isMachine1620Or2619or2615 ? "width=426,height=240" : "width=640,height=360";
    QString videoDecoder = isMachine1620Or2619or2615 ? "avdec_h264" : "v4l2h264dec";

    QProcess discoverer;
    discoverer.start("gst-discoverer-1.0", QStringList() << ("file://" + cleanFilePath));
    discoverer.waitForFinished();
    QString output = discoverer.readAllStandardOutput();
    bool hasAudio = false;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.trimmed().toLower().contains("audio #")) {
            hasAudio = true;
            break;
        }
    }

    if (hasAudio) {
        args << "filesrc" << ("location=" + cleanFilePath) << "!"
             << "qtdemux" << "name=demux"
             << "demux.video_0" << "!" << "queue" << "!" << "h264parse" << "!" << videoDecoder
             << "!" << "videoconvert" << "!" << "videoscale" << "!" << "videorate"
             << "!" << ("video/x-raw,format=I420," + videoResolution + ",framerate=30/1") << "!"
             << "webrtcsink" << "name=ws"
             << "demux.audio_0" << "!" << "queue" << "!" << "aacparse" << "!" << "avdec_aac"
             << "!" << "audioconvert" << "!" << "audioresample"
             << "!" << "audio/x-raw,format=S16LE,rate=48000,channels=2" << "!" << "ws.";
    } else {
        args << "filesrc" << ("location=" + cleanFilePath) << "!"
             << "qtdemux" << "name=demux"
             << "demux.video_0" << "!" << "queue" << "!" << "h264parse" << "!" << videoDecoder
             << "!" << "videoconvert" << "!" << "videoscale" << "!" << "videorate"
             << "!" << ("video/x-raw,format=I420," + videoResolution + ",framerate=30/1") << "!"
             << "webrtcsink";
    }

    fileSrc->start("gst-launch-1.0", args);
    if (!fileSrc->waitForStarted()) {
        delete fileSrc;
        fileSrc = nullptr;
        qWarning() << "Failed to start fileSrc process";
    } else {
        connect(fileSrc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitCode);
            Q_UNUSED(exitStatus);
            stopFileSrc();
            emit fileSrcEos();
        });
    }
}

void WebrtcManager::streamAIDemos(const QString &demoName)
{
    stopAIDemos();

    aiDemoSrc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GST_PLUGIN_FEATURE_RANK", "v4l2vp8enc:NONE");
    aiDemoSrc->setProcessEnvironment(env);

    QStringList args;
    bool isMachine1620 = getMachine().contains("1620");
    QString videoResolution = isMachine1620 ? "width=480,height=272" : "width=640,height=480";
    QString videoDecoder = isMachine1620 ? "avdec_h264" : "v4l2h264dec";
    QString inferenceResolution = isMachine1620 ? "width=256,height=160" : 
                             (demoName == "Face Detection" ? "width=640,height=480" : 
                             (demoName == "Object Detection" ? "width=640,height=384" : 
                             (demoName == "Pose Estimation" ? "width=640,height=352" : "width=640,height=480")));

    QString pipeline;
    if (demoName == "Face Detection") {
        QString cameraPath = getCameraPath();
        if (cameraPath.isEmpty()) {
            qWarning() << "No camera device found for Face Detection";
            delete aiDemoSrc;
            aiDemoSrc = nullptr;
            emit invalidFileSelected();
            return;
        }

        QString cameraSrc = isMachine1620 ? QString("v4l2src ! ") : QString("v4l2src device=%1 ! ").arg(cameraPath);
        QString modelPath = isMachine1620 ? 
                           "/usr/share/synap/models/object_detection/face/model.synap" :
                           "/usr/share/synap/models/object_detection/face/model/yolov5s_face_640x480_onnx_mq/model.synap";
        pipeline = cameraSrc +
                   "tee name=t_data "
                   "t_data. ! queue ! synavideoconvertscale ! video/x-raw," + inferenceResolution + ",format=RGB ! "
                   "synapinfer model=" + modelPath + " mode=detector frameinterval=" + (isMachine1620 ? "4" : "3") + " ! overlay.inference_sink "
                   "t_data. ! queue ! synavideoconvertscale ! video/x-raw," + videoResolution + ",format=BGRA ! "
                   "synapoverlay name=overlay ! "
                   "videoconvert ! videoscale ! videorate ! "
                   "video/x-raw,format=I420," + videoResolution + ",framerate=30/1 ! webrtcsink";
    } else if (demoName == "Object Detection") {
            if (isMachine1620) {
                QString cameraPath = getCameraPath();
            if (cameraPath.isEmpty()) {
                qWarning() << "No camera device found for Object Detection";
                delete aiDemoSrc;
                aiDemoSrc = nullptr;
                emit invalidFileSelected();
                return;
            }

            QString cameraSrc = isMachine1620 ? QString("v4l2src ! ") : QString("v4l2src device=%1 ! ").arg(cameraPath);
            pipeline = cameraSrc +
                       "tee name=t_data "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=256,height=160,format=RGB ! "
                       "synapinfer model=/usr/share/synap/models/object_detection/coco/model.synap mode=detector frameinterval=4 ! overlay.inference_sink "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! "
                       "synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! "
                       "videoconvert ! videoscale ! videorate ! "
                       "video/x-raw,format=I420," + videoResolution + ",framerate=30/1 ! webrtcsink";
        } else {
            QString cleanFilePath = "/home/root/demos/videos/mp4/object_detection.mp4";
            if (cleanFilePath.startsWith("file://")) {
                cleanFilePath = cleanFilePath.remove(0, 7);
            }

            if (!validateFile(cleanFilePath)) {
                qWarning() << "Invalid file for Object Detection: " << cleanFilePath;
                delete aiDemoSrc;
                aiDemoSrc = nullptr;
                emit invalidFileSelected();
                return;
            }

            pipeline = QString("filesrc location=%1 ! ").arg(cleanFilePath) +
                       "qtdemux name=demux "
                       "demux.video_0 ! queue ! h264parse ! " + videoDecoder + " ! "
                       "tee name=t_data "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw," + inferenceResolution + ",format=RGB ! "
                       "synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap "
                       "mode=detector frameinterval=3 ! overlay.inference_sink "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw," + videoResolution + " ! "
                       "synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! "
                       "webrtcsink";
        }
    } else if (demoName == "Pose Estimation") {
            if (isMachine1620) {
                        QString cameraPath = getCameraPath();
            if (cameraPath.isEmpty()) {
                qWarning() << "No camera device found for Pose Estimation";
                delete aiDemoSrc;
                aiDemoSrc = nullptr;
                emit invalidFileSelected();
                return;
            }

            QString cameraSrc = isMachine1620 ? QString("v4l2src ! ") : QString("v4l2src device=%1 ! ").arg(cameraPath);
            pipeline = cameraSrc +
                       "tee name=t_data "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=256,height=160,format=RGB ! "
                       "synapinfer model=/usr/share/synap/models/object_detection/body_pose/model.synap mode=detector frameinterval=4 ! overlay.inference_sink "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! "
                       "synapoverlay name=overlay ! "
                       "videoconvert ! videoscale ! videorate ! "
                       "video/x-raw,format=I420," + videoResolution + ",framerate=30/1 ! webrtcsink";
        } else {
            QString cleanFilePath = "/home/root/demos/videos/mp4/body_pose.mp4";
            if (cleanFilePath.startsWith("file://")) {
                cleanFilePath = cleanFilePath.remove(0, 7);
            }

            if (!validateFile(cleanFilePath)) {
                qWarning() << "Invalid file for Pose Estimation: " << cleanFilePath;
                delete aiDemoSrc;
                aiDemoSrc = nullptr;
                emit invalidFileSelected();
                return;
            }

            pipeline = QString("filesrc location=%1 ! ").arg(cleanFilePath) +
                       "qtdemux name=demux "
                       "demux.video_0 ! queue ! h264parse ! " + videoDecoder + " ! "
                       "tee name=t_data "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw," + inferenceResolution + ",format=RGB ! "
                       "synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap "
                       "mode=detector frameinterval=3 ! overlay.inference_sink "
                       "t_data. ! queue ! synavideoconvertscale ! video/x-raw," + videoResolution + ",format=BGRA ! "
                       "synapoverlay name=overlay ! "
                       "webrtcsink";
        }
    } else {
        qWarning() << "Unknown AI Demo: " << demoName;
        delete aiDemoSrc;
        aiDemoSrc = nullptr;
        emit invalidFileSelected();
        return;
    }

    args << pipeline.split(" ", Qt::SkipEmptyParts);

    aiDemoSrc->start("gst-launch-1.0", args);
    if (!aiDemoSrc->waitForStarted()) {
        qWarning() << "Failed to start aiDemoSrc process";
        delete aiDemoSrc;
        aiDemoSrc = nullptr;
        emit invalidFileSelected();
        return;
    } else {
        connect(aiDemoSrc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitCode);
            Q_UNUSED(exitStatus);
            stopAIDemos();
            emit aiDemoSrcEos();
        });
    }
}

QString WebrtcManager::getCameraPath()
{
    QProcess process;
    process.start("v4l2-ctl", QStringList() << "--list-devices");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);

    bool isUsbCamera = false;
    for (const QString &line : lines) {
        if (line.contains("usb", Qt::CaseInsensitive) && line.contains(":")) {
            isUsbCamera = true;
            continue;
        }
        if (isUsbCamera && line.contains("/dev/video")) {
            QString device = line.trimmed();
            QFile file(device);
            if (file.exists()) {
                return device;
            }
        }
        if (line.trimmed().isEmpty() || (!line.contains("/dev/") && line.contains(":"))) {
            isUsbCamera = false;
        }
    }

    return QString();
}

void WebrtcManager::streamCameraSrc(const QString &camPath)
{
    if (camPath.isEmpty()) {
        return;
    }

    stopFileSrc();
    stopCameraSrc();

    cameraSrc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GST_PLUGIN_FEATURE_RANK", "v4l2vp8enc:NONE");
    cameraSrc->setProcessEnvironment(env);

    QStringList args;
    args << "v4l2src" << ("device=" + camPath) << "!"
         << "video/x-raw,format=YUY2,width=640,height=480,framerate=30/1" << "!"
         << "videoconvert" << "!" << "videoscale" << "!" << "videorate" << "!"
         << "video/x-raw,format=I420,width=640,height=480,framerate=30/1" << "!"
         << "webrtcsink" << "name=ws";

    cameraSrc->start("gst-launch-1.0", args);
    if (!cameraSrc->waitForStarted()) {
        delete cameraSrc;
        cameraSrc = nullptr;
    }
}

void WebrtcManager::stopCameraSrc()
{
    if (cameraSrc) {
        disconnect(cameraSrc, nullptr, this, nullptr);
        cameraSrc->terminate();
        cameraSrc->waitForFinished(1000);
        cameraSrc->deleteLater();
        cameraSrc = nullptr;
    }
}