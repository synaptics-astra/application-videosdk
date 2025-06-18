// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QFileInfo>

class FileManager : public QObject {
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE bool copyFiles(const QStringList &sourcePaths, const QString &destDir) {
        QDir dir(destDir);
        if (!dir.exists()) {
            if (!dir.mkpath(destDir)) {
                qWarning() << "Failed to create directory:" << destDir;
                return false;
            }
            qDebug() << "Created directory:" << destDir;
        }

        bool success = true;
        for (const QString &srcPath : sourcePaths) {
            QString fileName = QFileInfo(srcPath).fileName();
            QString destPath = dir.filePath(fileName);
            if (QFile::copy(srcPath, destPath)) {
            } else {
                success = false;
            }
        }
        return success;
    }

    Q_INVOKABLE bool deleteFiles(const QStringList &filePaths) {
        bool success = true;
        for (const QString &filePath : filePaths) {
            if (QFile::remove(filePath)) {
                qDebug() << "Deleted" << filePath;
            } else {
                qWarning() << "Failed to delete" << filePath << ":" << QFile(filePath).errorString();
                success = false;
            }
        }
        return success;
    }
};

#endif // FILEMANAGER_H
