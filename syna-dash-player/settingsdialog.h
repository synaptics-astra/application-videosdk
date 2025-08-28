// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "filepopup.h"
#include "samplesrcpopup.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDebug>
#include <QNetworkConfigurationManager>

class FilePopup;
class SampleSrcPopup;
class SettingsPopup : public QWidget {
    Q_OBJECT
public:
    SettingsPopup(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::Popup);
        setStyleSheet("background-color: #839192; font-family: Arial;");
        setFixedSize(600, 200);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(30, 30, 30, 30);
        mainLayout->setSpacing(25);

        QLabel *settingsLabel = new QLabel("Input Config", this);
        QFont titleFont;
        titleFont.setPixelSize(24);
        settingsLabel->setFont(titleFont);
        settingsLabel->setStyleSheet("font-weight: bold; color: #1d1d1d;");

        QHBoxLayout *inputLayout = new QHBoxLayout;
        inputLayout->setContentsMargins(0, 0, 0, 0);
        inputLayout->setSpacing(15);

        QLabel *inputLabel = new QLabel("Input Source:", this);
        QFont labelFont;
        labelFont.setPixelSize(14);
        inputLabel->setFont(labelFont);
        inputLabel->setStyleSheet("color: #1d1d1d;");

        inputComboBox = new QComboBox(this);
        inputComboBox->setFont(labelFont);
        inputComboBox->setFixedSize(200, 40);
        inputComboBox->addItem("File Source");
        inputComboBox->addItem("URL");

        inputLayout->addWidget(inputLabel);
        inputLayout->addWidget(inputComboBox);

        QPushButton *saveButton = new QPushButton("Save", this);
        saveButton->setFont(labelFont);
        saveButton->setFixedSize(120, 50);
        saveButton->setStyleSheet("background-color: #0078D7; color: #fff; border: none; padding: 10px; border-radius: 8px;");
        saveButton->setCursor(Qt::PointingHandCursor);

        mainLayout->addWidget(settingsLabel);
        mainLayout->addLayout(inputLayout);
        mainLayout->addWidget(saveButton);

        filePopup = new FilePopup(this);
        sampleSrcPopup = new SampleSrcPopup(this);

        connect(filePopup, &FilePopup::fileSelected, this, [this](const QString &filePath) {
            QString inputType = "File Source";
            QString inputSource = filePath;

            if (inputSource.isEmpty()) {
                return;
            }

            emit settingsChanged(inputType, inputSource);
            hide();
        });

        connect(sampleSrcPopup, &SampleSrcPopup::urlSelected, this, [this](const QString &url) {
            QString inputType = "URL";
            QString inputSource = url;

            if (inputSource.isEmpty()) {
                return;
            }

            emit settingsChanged(inputType, inputSource);
            hide();
        });

        connect(saveButton, &QPushButton::clicked, this, [this]() {
            QString inputType = inputComboBox->currentText();
            if (inputType == "File Source") {
                filePopup->showPopup(QCursor::pos());
            } else if (inputType == "URL") {
                QNetworkConfigurationManager mgr;
                bool isOnline = mgr.isOnline();
                if (!isOnline) {
                    ValidationPopup *validationPopup = new ValidationPopup("Please check your internet connection and try again.", this);
                    validationPopup->setAttribute(Qt::WA_DeleteOnClose);
                    QPoint popupPos = QCursor::pos() + QPoint(10, 10);
                    validationPopup->showPopup(popupPos);
                    validationPopup->raise();
                    validationPopup->activateWindow();
                    return;
                }
                sampleSrcPopup->showPopup(QCursor::pos());
            }
        });
    }

signals:
    void settingsChanged(const QString &inputType, const QString &inputSource);

public slots:
    void showPopup(const QPoint &pos) {
        move(pos);
        show();
    }

private:
    QComboBox *inputComboBox;
    FilePopup *filePopup;
    SampleSrcPopup *sampleSrcPopup;
};

#endif // SETTINGSDIALOG_H