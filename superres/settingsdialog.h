// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "qtdevice.h"
#include "videoprober.h"
#include "filedialoghelper.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

class QtDevice;
class SettingsPopup : public QWidget {
    Q_OBJECT
public:
    SettingsPopup(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::Popup);
        setStyleSheet("background-color: #839192; font-family: Arial;");
        setFixedSize(600, 500);

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
        QtDevice *device = new QtDevice();
        QVector<QVector<std::string>> qtDevices = device->getDevicePath();
        for (const auto& row : qtDevices) {
            if (row.size() >= 2) {
                inputComboBox->addItem(QString::fromStdString(row[0]));
            }
        }
        delete device;

        inputLayout->addWidget(inputLabel);
        inputLayout->addWidget(inputComboBox);

        inputResComboBox = new QComboBox(this);
        inputResComboBox->setFont(labelFont);
        inputResComboBox->setFixedSize(200, 40);
        inputResComboBox->addItem("640x360");

        outputResComboBoxHD = new QComboBox(this);
        outputResComboBoxHD->setFont(labelFont);
        outputResComboBoxHD->setFixedSize(200, 40);
        outputResComboBoxHD->addItem("1920x1080");

        outputResComboBox4k = new QComboBox(this);
        outputResComboBox4k->setFont(labelFont);
        outputResComboBox4k->setFixedSize(200, 40);
        outputResComboBox4k->addItem("3840x2160");

        QVBoxLayout *inputResLayout = new QVBoxLayout;
        QLabel *inputResLabel = new QLabel("Input Resolution:", this);
        inputResLabel->setFont(labelFont);
        inputResLayout->addWidget(inputResLabel);
        inputResLayout->addWidget(inputResComboBox);

        QVBoxLayout *outputResLayout = new QVBoxLayout;
        QLabel *outputResLabel = new QLabel("Output Resolution:", this);
        outputResLabel->setFont(labelFont);
        outputResLayout->addWidget(outputResLabel);
        outputResLayout->addWidget(outputResComboBoxHD);
        outputResLayout->addWidget(outputResComboBox4k);

        outputResComboBox4k->hide();

        QHBoxLayout *radioLayout = new QHBoxLayout;
        radioLayout->setContentsMargins(0, 0, 0, 0);
        radioLayout->setSpacing(15);

        QLabel *engineLabel = new QLabel("Model Type:", this);
        engineLabel->setFont(labelFont);
        engineLabel->setStyleSheet("color: #1d1d1d;");

        QRadioButton *fastRadioButton = new QRadioButton("FAST", this);
        fastRadioButton->setFont(labelFont);
        fastRadioButton->setChecked(true);
        QRadioButton *qdeoRadioButton = new QRadioButton("QDEO", this);
        qdeoRadioButton->setFont(labelFont);

        QButtonGroup *engineButtonGroup = new QButtonGroup(this);
        engineButtonGroup->addButton(fastRadioButton);
        engineButtonGroup->addButton(qdeoRadioButton);

        radioLayout->addWidget(engineLabel);
        radioLayout->addWidget(fastRadioButton);
        radioLayout->addWidget(qdeoRadioButton);

        connect(inputComboBox, &QComboBox::currentTextChanged, this, &SettingsPopup::onInputTypeChanged);

        connect(inputResComboBox, &QComboBox::currentTextChanged, [this, fastRadioButton, qdeoRadioButton](const QString &text) {
            qDebug() << "Input resolution changed to:" << text;
            outputResComboBoxHD->hide();
            outputResComboBox4k->hide();
            if (text == "640x360") {
                outputResComboBoxHD->show();
            } else {
                outputResComboBox4k->show();
            }
            fastRadioButton->setEnabled(true);
            qdeoRadioButton->setEnabled(true);
        });

        QHBoxLayout *comboBoxLayout = new QHBoxLayout;
        comboBoxLayout->addLayout(inputResLayout);
        comboBoxLayout->addLayout(outputResLayout);

        QPushButton *saveButton = new QPushButton("Save", this);
        saveButton->setFont(labelFont);
        saveButton->setFixedSize(120, 50);
        saveButton->setStyleSheet("background-color: #0078D7; color: #fff; border: none; padding: 10px; border-radius: 8px;");
        saveButton->setCursor(Qt::PointingHandCursor);

        mainLayout->addWidget(settingsLabel);
        mainLayout->addLayout(inputLayout);
        mainLayout->addLayout(comboBoxLayout);
        mainLayout->addLayout(radioLayout);
        mainLayout->addWidget(saveButton);

        connect(saveButton, &QPushButton::clicked, this, [this, qtDevices, qdeoRadioButton, fastRadioButton]() {
            QString node = inputComboBox->currentText();
            QString inputType;
            QString inputSource;
            QString upscalingType;
            QString modelType = qdeoRadioButton->isChecked() ? "QDEO" : "FAST";

            if (node == "File Source") {
                inputType = "File Source";
                FileDialogHelper dialogHelper;
                QVariantList filters;
                filters << "*.mp4" << "*.avi" << "*.mkv";
                inputSource = dialogHelper.selectSingleFile("Select Video File", filters, "/home/root");
                if (inputSource.isEmpty()) {
                    return;
                }
            } else {
                for (const auto& row : qtDevices) {
                    if (row.size() >= 2 && QString::fromStdString(row[0]) == node) {
                        inputType = QString::fromStdString(row[0]);
                        inputSource = QString::fromStdString(row[1]);
                    }
                }
                QString inputRes = inputResComboBox->currentText();
                QString outputRes = (inputRes == "640x360") ? "1920x1080" : outputResComboBox4k->currentText();
                upscalingType = inputRes + "_" + outputRes;
            }

            emit settingsChanged(inputType, inputSource, upscalingType, modelType);
            hide();
        });

        onInputTypeChanged(inputComboBox->currentText());
    }

signals:
    void settingsChanged(const QString &inputType, const QString &inputSource, const QString &upscalingType, const QString &modelType);

public slots:
    void showPopup(const QPoint &pos) {
        move(pos);
        show();
    }

private slots:
    void onInputTypeChanged(const QString &text) {
        inputResComboBox->blockSignals(true);
        inputResComboBox->clear();
        bool isFileSource = (text == "File Source");
        if (isFileSource) {
            inputResComboBox->addItem("640x360");
            inputResComboBox->addItem("960x540");
            inputResComboBox->addItem("1280x720");
            inputResComboBox->setEnabled(false);
            outputResComboBoxHD->setEnabled(false);
            outputResComboBox4k->setEnabled(false);
        } else {
            inputResComboBox->addItem("640x360");
            inputResComboBox->addItem("960x540");
            inputResComboBox->addItem("1280x720");
            inputResComboBox->setEnabled(true);
            outputResComboBoxHD->setEnabled(true);
            outputResComboBox4k->setEnabled(true);
        }
        inputResComboBox->setCurrentIndex(0);
        inputResComboBox->blockSignals(false);
        emit inputResComboBox->currentTextChanged(inputResComboBox->currentText());
    }

private:
    QComboBox *inputComboBox;
    QComboBox *inputResComboBox;
    QComboBox *outputResComboBoxHD;
    QComboBox *outputResComboBox4k;
};

#endif // SETTINGSDIALOG_H
