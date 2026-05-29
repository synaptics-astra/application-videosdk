// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef CAMERAIMAGEPROVIDER_H
#define CAMERAIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QDebug>

class CameraImageProvider : public QQuickImageProvider {
public:
    explicit CameraImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void updateImage(const QImage &image);

private:
    QImage currentImage;
    QMutex mutex;
};

#endif // CAMERAIMAGEPROVIDER_H
