// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef QT_DEVICE_H
#define QT_DEVICE_H

#include <QString>
#include <QVector>
#include <QDebug>

#include <string.h>
#include <cstdlib>
#include <libudev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/videodev2.h>

using namespace std;

class QtDevice {
    public:
        QVector<QVector<string>> getDevicePath();
    private:
        QVector<QVector<string>> qtDevices;
};
#endif // QT_DEVICE_H
