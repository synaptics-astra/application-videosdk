#include "filedialoghelper.h"

FileDialogHelper::FileDialogHelper(QObject *parent)
    : QObject(parent)
{
}

QStringList FileDialogHelper::getFilesFromDirectory(const QString &dirPath, const QString &extension)
{
    QDir dir(dirPath);
    QStringList fileNames;

    if (dir.exists()) {
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &file : files) {
            if (!(file.isFile() && (file.suffix() != extension))) {
                fileNames.append(file.fileName());
            }
        }
    }
    return fileNames;
}

QString FileDialogHelper::getParentDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    if (dir.cdUp()) {
        return dir.absolutePath();
    }
    return dirPath;
}

bool FileDialogHelper::openFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = file.readAll();
        emit fileOpened(filePath);
        file.close();
        return true;
    }
    return false;
}

bool FileDialogHelper::isDirectory(const QString &path)
{
    QFileInfo fileInfo(path);
    return fileInfo.isDir();
}
