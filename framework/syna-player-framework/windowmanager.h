// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <qpa/qplatformnativeinterface.h>
#include <gst/wayland/wayland.h>

#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QMutex>
#include <QPushButton>
#include <QVector>
#include <QPalette>
#include <QString>
#include <iterator>

#include <string.h>
#include <cstdlib>
#include <libudev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/videodev2.h>

#include "gstplayer.h"

#define SL1620_ASTRA_MACHINE "sl1620"
#define SL1640_ASTRA_MACHINE "sl1640"
#define SL1680_ASTRA_MACHINE "sl1680"
#define USE_SOFTWARE_DECODER "ffmpeg"
#define USE_HARDWARE_DECODER "v4l2"

using namespace std;

class GstPlayer;
struct v4l2dev_t
{
    int dev_fd;
    uint32_t dev_v4l2_fmt;
};
class WindowManager : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString cameraFormat READ cameraFormat NOTIFY cameraFormatChanged)

public:
    explicit WindowManager(QWidget *parent = nullptr);
    static QString CONFIG_FILE_PATH;

    QString cameraFormat() const { return cameraFormatList; }
    QString hrxFormat() const { return hrxFormatList; }
    void updateCAMSRC(QVector<QString>::iterator iter);
    void updateHRXSRC(QVector<QString>::iterator iter);
    void updateRTSPSRC(QVector<QString>::iterator iter);
    ~WindowManager() {}
    Q_INVOKABLE void play(QString url1, QString url2);
    Q_INVOKABLE void play(int index, QString url1, QString url2,
            QString url3, QString url4);
    static void createConfig();
    GstContext* context;

public slots:
    void threadFinished();

private:
    int findScaleFactor(int a, int b);
    void getDevicePath();
    void getHRXDevicePath();
    int index{-1}, width{-1}, height{-1}, scaleFactor{-1};
    int threadCount{0};
    int camCount;
    int hrxCount;
    QMutex mutex;
    QVector<QString> urls;
    QVector<QString>::iterator iter;
    QVector<GstPlayer*> players;
    QPushButton *closeButton;
    QVector<string> cams;
    QVector<string>camsfmt;
    QVector<string> hrxs;
    QVector<string>::iterator camIter;
    QVector<string>::iterator camsfmtIter;
    QVector<string>::iterator hrxIter;
    QString cameraFormatList = "YUY2";
    QString hrxFormatList = "NV12";

signals:
    void cameraFormatChanged();
    void exitThread();

private slots:
    void cleanUp();
};
#endif // WINDOW_MANAGER_H

