// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef FILEPOPUP_H
#define FILEPOPUP_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QDebug>
#include <QDir>
#include "videoprober.h"
#include "validationpopup.h"

class ValidationPopup;
class FilePopup : public QWidget {
    Q_OBJECT
public:
    FilePopup(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::Popup);
        setStyleSheet("background-color: #ffffff; font-family: Arial;");
        setFixedSize(600, 400);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(10);

        QLabel *titleLabel = new QLabel("Select a Video File", this);
        QFont titleFont;
        titleFont.setPixelSize(18);
        titleLabel->setFont(titleFont);
        titleLabel->setStyleSheet("color: black; font-weight: bold;");

        pathField = new QLineEdit(this);
        pathField->setReadOnly(true);
        pathField->setFont(QFont("Arial", 14));
        pathField->setStyleSheet("background-color: #f0f0f0; border: 1px solid #cccccc; border-radius: 4px; padding: 5px;");

        fileModel = new QFileSystemModel(this);
        fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
        fileModel->setNameFilters({"*.h264", "*.mp4"});
        fileModel->setNameFilterDisables(false);
        fileModel->setRootPath(QDir::homePath());

        fileList = new QListView(this);
        fileList->setModel(fileModel);
        fileList->setRootIndex(fileModel->index(QDir::homePath()));
        fileList->setFont(QFont("Arial", 14));
        fileList->setStyleSheet("border: 1px solid #666666; border-radius: 4px;");

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *backButton = new QPushButton("Back", this);
        backButton->setFont(QFont("Arial", 14));
        backButton->setFixedSize(120, 40);
        backButton->setStyleSheet(
            "QPushButton { background-color: #003366; color: white; border: none; padding: 10px; border-radius: 8px; }"
            "QPushButton:disabled { background-color: #999999; border: 1px solid #777777; }"
            "QPushButton:hover { background-color: #005999; }"
            "QPushButton:pressed { background-color: #003366; }");

        QPushButton *cancelButton = new QPushButton("Cancel", this);
        cancelButton->setFont(QFont("Arial", 14));
        cancelButton->setFixedSize(120, 40);
        cancelButton->setStyleSheet(
            "QPushButton { background-color: #003366; color: white; border: none; padding: 10px; border-radius: 8px; }"
            "QPushButton:hover { background-color: #005999; }"
            "QPushButton:pressed { background-color: #003366; }");

        buttonLayout->addStretch();
        buttonLayout->addWidget(backButton);
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addStretch();

        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(pathField);
        mainLayout->addWidget(fileList);
        mainLayout->addLayout(buttonLayout);

        connect(fileList, &QListView::clicked, this, &FilePopup::onItemClicked);
        connect(backButton, &QPushButton::clicked, this, &FilePopup::onBackClicked);
        connect(cancelButton, &QPushButton::clicked, this, &FilePopup::hide);

        updatePathField();
    }

signals:
    void fileSelected(const QString &filePath);

public slots:
    void showPopup(const QPoint &pos) {
        move(pos);
        fileModel->setRootPath(QDir::homePath());
        fileList->setRootIndex(fileModel->index(QDir::homePath()));
        updatePathField();
        show();
    }

private slots:
    void onItemClicked(const QModelIndex &index) {
        QString path = fileModel->filePath(index);
        QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            fileModel->setRootPath(path);
            fileList->setRootIndex(fileModel->index(path));
            updatePathField();
        } else {
            QString suffix = fileInfo.suffix().toLower();
            VideoProber prober;
            QVariantMap videoInfo = prober.probeFile(path);
            QString codec = videoInfo.value("codec").toString().toLower();
            int videoWidth = videoInfo.value("width").toInt();
            int videoHeight = videoInfo.value("height").toInt();
            bool isValid = false;
            QString errorMessage;

            if (suffix == "h264") {
                if (codec.contains("h264") || codec.contains("h.264")) {
                    if (videoWidth <= 1280 && videoHeight <= 720) {
                        isValid = true;
                    } else {
                        errorMessage = QString("Only video resolutions up to 720p are supported. Your video resolution is %1x%2.").arg(videoWidth).arg(videoHeight);
                    }
                } else {
                    errorMessage = "Only H.264 encoded .h264 files are supported.";
                }
            } else if (suffix == "mp4") {
                if (codec.contains("h264") || codec.contains("h.264")) {
                    if (videoWidth <= 1280 && videoHeight <= 720) {
                        isValid = true;
                    } else {
                        errorMessage = QString("Only video resolutions up to 720p are supported. Your video resolution is %1x%2.").arg(videoWidth).arg(videoHeight);
                    }
                } else if (codec.contains("h265") || codec.contains("hevc")) {
                    errorMessage = "Only H.264 encoded .mp4 files are supported. The selected .mp4 file uses H.265/HEVC codec.";
                } else {
                    errorMessage = "Only H.264 encoded .mp4 files are supported.";
                }
            } else {
                errorMessage = "Only .mp4 and .h264 files with H.264 codec are supported.";
            }

            if (isValid) {
                emit fileSelected(path);
                hide();
            } else {
                ValidationPopup *validationPopup = new ValidationPopup(errorMessage, this);
                validationPopup->showPopup(QCursor::pos());
            }
        }
    }

    void onBackClicked() {
        QDir currentDir(fileModel->rootPath());
        if (currentDir.cdUp()) {
            fileModel->setRootPath(currentDir.path());
            fileList->setRootIndex(fileModel->index(currentDir.path()));
            updatePathField();
        }
    }

private:
    void updatePathField() {
        pathField->setText(fileModel->rootPath());
    }

    QLineEdit *pathField;
    QListView *fileList;
    QFileSystemModel *fileModel;
};

#endif // FILEPOPUP_H