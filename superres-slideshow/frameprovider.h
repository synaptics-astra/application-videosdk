// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class FrameProvider : public QQuickImageProvider
{
public:
    FrameProvider();

    QImage requestImage(const QString &id,
                        QSize *size,
                        const QSize &requestedSize) override;

    // called by controller when a new frame is ready
    void updateNormalImage(const QImage &image);
    void updateSrImage(const QImage &image);

private:
    QImage normalImage;
    QImage srImage;
    QMutex imageMutex;
};

#endif // FRAMEPROVIDER_H
