// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef VIDEOPROBER_H
#define VIDEOPROBER_H

#include <QObject>
#include <QProcess>
#include <QVariantMap>

class VideoProber : public QObject {
    Q_OBJECT

public:
    explicit VideoProber(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap probeFile(const QString &filePath);

private:
    QVariantMap parseGstDiscovererOutput(const QString &output);
};

#endif // VIDEOPROBER_H
