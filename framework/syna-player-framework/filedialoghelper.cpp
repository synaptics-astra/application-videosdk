// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "filedialoghelper.h"

#include <QFileDialog>
#include <QFileIconProvider>
#include <QIcon>
#include <QUrl>

namespace {

class SharedDialogIconProvider : public QFileIconProvider
{
public:
    QIcon icon(const QFileInfo &info) const override
    {
        if (info.isDir()) {
            QIcon folderIcon(QStringLiteral(":/images/folder_icon.png"));
            if (!folderIcon.isNull()) {
                return folderIcon;
            }
        } else {
            QIcon fileIcon(QStringLiteral(":/images/play_round_icon.png"));
            if (!fileIcon.isNull()) {
                return fileIcon;
            }
        }

        return QFileIconProvider::icon(info);
    }
};

QStringList normalizeNameFilters(const QVariantList &filters)
{
    QStringList directFilters;
    QStringList patternFilters;

    for (const QVariant &entry : filters) {
        const QString filter = entry.toString().trimmed();
        if (filter.isEmpty()) {
            continue;
        }

        if (filter.contains(QLatin1Char('(')) && filter.contains(QLatin1Char(')'))) {
            directFilters << filter;
            continue;
        }

        if (filter.contains(QLatin1Char('*'))) {
            patternFilters << filter;
        } else if (filter.contains(QLatin1Char('.'))) {
            patternFilters << QStringLiteral("*") + filter;
        } else {
            patternFilters << QStringLiteral("*.") + filter;
        }
    }

    QStringList outputFilters;
    if (!patternFilters.isEmpty()) {
        outputFilters << QStringLiteral("Supported Files (%1)").arg(patternFilters.join(QLatin1Char(' ')));
    }

    outputFilters << directFilters;

    if (outputFilters.isEmpty()) {
        outputFilters << QStringLiteral("All Files (*)");
    }

    return outputFilters;
}

QString resolveStartDirectory(const QString &startDirectory)
{
    if (startDirectory.startsWith(QStringLiteral("file://"))) {
        return QUrl(startDirectory).toLocalFile();
    }
    return startDirectory;
}

void applySharedStyle(QFileDialog &dialog)
{
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setLabelText(QFileDialog::Accept, QStringLiteral("Select"));
    dialog.setLabelText(QFileDialog::Reject, QStringLiteral("Cancel"));

    auto *iconProvider = new SharedDialogIconProvider();
    dialog.setIconProvider(iconProvider);

    dialog.setStyleSheet(
        "QFileDialog { background: #0069d9; border: 2px solid black; }"
        "QFileDialog QLabel { color: #000000; font-weight: bold; }"
        "QLineEdit { background: white; color: #222222; border: 1px solid black; min-height: 28px; }"
        "QListView, QTreeView { background: white; color: #222222; border: 1px solid black; alternate-background-color: #f8f8f8; }"
        "QListView::item:selected, QTreeView::item:selected { background: #d9ecff; color: #222222; }"
        "QPushButton { min-height: 30px; padding: 4px 10px; }"
    );
}

} // namespace

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

QString FileDialogHelper::selectSingleFile(const QString &title,
                                           const QVariantList &filters,
                                           const QString &startDirectory)
{
    QFileDialog dialog(nullptr, title, resolveStartDirectory(startDirectory));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters(normalizeNameFilters(filters));
    applySharedStyle(dialog);

    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return QString();
    }

    return dialog.selectedFiles().first();
}

QStringList FileDialogHelper::selectMultipleFiles(const QString &title,
                                                  const QVariantList &filters,
                                                  const QString &startDirectory)
{
    QFileDialog dialog(nullptr, title, resolveStartDirectory(startDirectory));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilters(normalizeNameFilters(filters));
    applySharedStyle(dialog);

    if (dialog.exec() != QDialog::Accepted) {
        return QStringList();
    }

    return dialog.selectedFiles();
}
