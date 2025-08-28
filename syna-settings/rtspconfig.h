// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef RTSPCONFIG_H
#define RTSPCONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <QDebug>

class RtspConfig : public QObject {
    Q_OBJECT

public:
    explicit RtspConfig(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap validateRtspUrl(const QString &url);
    Q_INVOKABLE QVariantList validateAllRtspUrls(const QStringList &urls);
    Q_INVOKABLE bool checkFileExists(const QString &filePath);

};

#endif // RTSPCONFIG_H
