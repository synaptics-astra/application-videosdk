// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "videoprober.h"
#include <QDebug>

VideoProber::VideoProber(QObject *parent) : QObject(parent) {
}

QVariantMap VideoProber::probeFile(const QString &filePath) {
    QProcess process;
    process.start("gst-discoverer-1.0", QStringList() << "-v" << filePath);
    process.waitForFinished(5000);

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qWarning() << "gst-discoverer-1.0 failed:" << process.errorString();
        return QVariantMap();
    }

    QString output = QString::fromUtf8(process.readAllStandardOutput());

    return parseGstDiscovererOutput(output);
}

QVariantMap VideoProber::parseGstDiscovererOutput(const QString &output) {
    QVariantMap result;
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    int width = 0, height = 0, depth = 0;
    QString codec, container, framerate = "";

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();

        // Width
        if (trimmed.startsWith("Width:", Qt::CaseInsensitive)) {
            width = trimmed.section(':', 1).trimmed().toInt();
        }
        // Height
        else if (trimmed.startsWith("Height:", Qt::CaseInsensitive)) {
            height = trimmed.section(':', 1).trimmed().toInt();
        }
        // Depth
        else if (trimmed.startsWith("Depth:", Qt::CaseInsensitive)) {
            depth = trimmed.section(':', 1).trimmed().toInt();
        }
        // Frame rate
        else if (trimmed.startsWith("Frame rate:", Qt::CaseInsensitive)) {
            framerate = trimmed.section(':', 1).trimmed();
        }
        // Codec detection
        else if (trimmed.contains("video/x-h264", Qt::CaseInsensitive)) {
            codec = "h264";
        }
        else if (trimmed.contains("video/x-h265", Qt::CaseInsensitive)) {
            codec = "h265";
        }
    }

    // container detection logic
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();

        if (trimmed.startsWith("container format:", Qt::CaseInsensitive)) {
            QString format = trimmed.section(':', 1).trimmed();

            if (format.contains("quicktime", Qt::CaseInsensitive) || format.contains("ISO MP4", Qt::CaseInsensitive))
                container = "mp4";
        }
        // Fallback container detection from MIME in "container #" line
        else if (trimmed.contains("container #", Qt::CaseInsensitive)) {
            QString lowerTrimmed = trimmed.toLower();

            if (lowerTrimmed.contains("video/x-quicktime"))
                container = "mp4";
        }
    }

    result["width"] = width;
    result["height"] = height;
    result["depth"] = depth;
    result["codec"] = codec;
    result["container"] = container;
    result["framerate"] = framerate;

    return result;
}