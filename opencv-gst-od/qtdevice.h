// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#ifndef QTDEVICE_H
#define QTDEVICE_H

#include <QString>
#include <QVector>

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
#endif // QTDEVICE_H
