// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <qpa/qplatformnativeinterface.h>

#ifdef HAVE_WAYLAND
#include <gst/wayland/wayland.h>
#endif

#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QMutex>
#include <QPushButton>
#include <QVector>
#include <QPalette>
#include <QString>
#include <iterator>
#include <QThread>
#include <QSemaphore>
#include <QDir>
#include <QRecursiveMutex>

#include <string.h>
#include <cstdlib>
#include <libudev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/videodev2.h>

#include "gstplayer.h"
#include "settingsmanager.h"
#include "statswidget.h"
#include "syna-player-common.h"

using namespace std;

class GstPlayer;
struct v4l2dev_t
{
    int dev_fd;
    uint32_t dev_v4l2_fmt;
};

struct camInfo {
    string cam;
    string fmt;
    string type;
    bool isUsed = false;
};

class WindowManager : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString cameraFormat READ cameraFormat NOTIFY cameraFormatChanged)

public:
    explicit WindowManager(QWidget *parent = nullptr, string mach = "", QString appName = "");

    QString cameraFormat() const { return cameraFormatList; }
    QString hrxFormat() const { return hrxFormatList; }
    void updateCAMSRC(QVector<QString>::iterator iter);
    void updateHRXSRC(QVector<QString>::iterator iter);
    void updateRTSPSRC(QVector<QString>::iterator iter);
    void playPipelines();
    ~WindowManager() {}
    Q_INVOKABLE void play(QString url1, QString url2);
    Q_INVOKABLE void play(int index, QString url1, QString url2,
            QString url3, QString url4);
    GstContext* context;
    bool hasLinuxDmabuf() const { return linuxDmabufAvailable; }

public slots:
    void threadFinished();
    void requestStatsClose();

private:
    int findScaleFactor(int a, int b);
    void getDevicePath();
    void getHRXDevicePath();
    int index{-1}, width{-1}, height{-1};
    float scaleFactor{-1.0f};
    int threadCount{0};
    int camCount;
    int hrxCount;
    int rtspIndex = 0;
    QRecursiveMutex mutex;
    QVector<QString> urls;
    QString getVideoDeviceNameFromSysPath(const char* syspath);
    bool isUsbDevice(struct udev_device *dev);
    QVector<QString>::iterator iter;
    QVector<GstPlayer*> players;
    QVector<camInfo> camInfoList;
    QVector<string> hrxs;
    QVector<string>::iterator hrxIter;
    QString cameraFormatList = "YUY2";
    QString hrxFormatList = "NV12";
    bool linuxDmabufAvailable = false;
    bool isPipelinePlaying = false;
    QSemaphore playPipelineSem;
    QThread* playPipelineThread = nullptr;
    string machine;
    SettingsManager* settingsManager;
    StatsWidget *statsWidget = nullptr;
    QString appName = "";

signals:
    void cameraFormatChanged();
    void exitThread();

private slots:
    void cleanUp();
};
#endif // WINDOW_MANAGER_H

