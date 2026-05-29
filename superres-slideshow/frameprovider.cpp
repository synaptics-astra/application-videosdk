// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "frameprovider.h"

FrameProvider::FrameProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage FrameProvider::requestImage(const QString &id,
                                   QSize *size,
                                   const QSize &requestedSize)
{
    QString cleanId = id.split("?")[0];

    QMutexLocker locker(&imageMutex);
    QImage img;

    if (cleanId == "normalFrame") {
        img = normalImage;
    } else if (cleanId == "srFrame") {
        img = srImage;
    }

    if (size)
     *size = img.size();

    if (!requestedSize.isEmpty()) {
        return img.scaled(requestedSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }

    return img;
}

void FrameProvider::updateNormalImage(const QImage &image)
{
    QMutexLocker locker(&imageMutex);
    normalImage = image;
}

void FrameProvider::updateSrImage(const QImage &image)
{
    QMutexLocker locker(&imageMutex);
    srImage = image;
}
