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

#include "gstplayer.h"

using namespace std;

class GstPlayer;
class WindowManager: public QWidget {
    Q_OBJECT
    public:
        WindowManager();
        ~WindowManager() {}
        Q_INVOKABLE void play(int index, QString url1, QString url2,
                QString url3, QString url4);
        void threadFinished();

        GstContext* context;

    private:
        int findScaleFactor(int a, int b);
        void getDevicePath();

        int index, width, height, scaleFactor;
        int threadCount{0};
        int camCount;
        QMutex mutex;
        QVector<QString> urls;
        QVector<QString>::iterator iter;
        QVector<GstPlayer*> players;
        QPushButton *closeButton;
        QVector<string> cams;
        QVector<string>::iterator camIter;

    signals:
        void exitThread();

    private slots:
        void cleanUp();

};
#endif // WINDOW_MANAGER_H
