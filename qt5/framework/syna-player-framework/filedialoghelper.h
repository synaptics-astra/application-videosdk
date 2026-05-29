// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef FILEDIALOGHELPER_H
#define FILEDIALOGHELPER_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QFile>

class FileDialogHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileDialogHelper(QObject *parent = nullptr);

    Q_INVOKABLE QStringList getFilesFromDirectory(const QString &dirPath, const QString &extension);
    Q_INVOKABLE QString getParentDirectory(const QString &dirPath);
    Q_INVOKABLE bool openFile(const QString &filePath);
    Q_INVOKABLE bool isDirectory(const QString &path);

signals:
    void fileOpened(const QString &filePath);

};

#endif // FILEDIALOGHELPER_H
