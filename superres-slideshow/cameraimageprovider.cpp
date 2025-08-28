// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "cameraimageprovider.h"

CameraImageProvider::CameraImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    currentImage = QImage(640, 480, QImage::Format_RGB888);
    currentImage.fill(Qt::black);
}

QImage CameraImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QMutexLocker locker(&mutex);
    if (!id.contains("preview")) {
        qWarning() << "Invalid image ID:" << id;
        return QImage();
    }

    QImage image = currentImage;
    if (size) {
        *size = image.size();
    }
    if (!requestedSize.isEmpty()) {
        image = image.scaled(requestedSize, Qt::KeepAspectRatio);
    }
    return image;
}

void CameraImageProvider::updateImage(const QImage &image)
{
    QMutexLocker locker(&mutex);
    currentImage = image;
}
