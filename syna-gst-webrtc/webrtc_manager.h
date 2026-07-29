// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef WEBRTC_MANAGER_H
#define WEBRTC_MANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>

class WebrtcManager : public QObject {
    Q_OBJECT

public:
    explicit WebrtcManager(QObject *parent = nullptr);
    ~WebrtcManager();

    Q_INVOKABLE void startServers();
    Q_INVOKABLE void stopServers();
    Q_INVOKABLE void streamFileSrc(const QString &filePath);
    Q_INVOKABLE void streamAIDemos(const QString &filePath);
    Q_INVOKABLE void streamCameraSrc(const QString &camPath);
    Q_INVOKABLE QString getPreferredIP();
    Q_INVOKABLE void stopFileSrc();
    Q_INVOKABLE void stopAIDemos();
    Q_INVOKABLE void stopCameraSrc();
    Q_INVOKABLE QString getCameraPath();
    Q_INVOKABLE QString getMachine();
    Q_INVOKABLE bool validateFile(const QString &filePath);
    Q_INVOKABLE bool hasInternetConnection();

signals:
    void fileSrcEos();
    void aiDemoSrcEos();
    void invalidFileSelected();
    void noInternetConnection();

private:
    QProcess *signallingServer;
    QProcess *httpServer;
    QProcess *fileSrc;
    QProcess *aiDemoSrc;
    QProcess *cameraSrc;
};

#endif // WEBRTC_MANAGER_H