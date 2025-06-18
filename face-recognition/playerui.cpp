// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "playerui.h"

PlayerUI::PlayerUI(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(60);
    headerWidget->setStyleSheet("background-color: #007dc3;");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);

    QLabel *iconLabel = new QLabel(headerWidget);
    iconLabel->setPixmap(QPixmap(":/images/astra_icon.png").scaled(50, 50));

    QLabel *titleLabel = new QLabel("Face ID", headerWidget);
    QFont titleFont;
    titleFont.setPixelSize(24);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white;");

    headerLayout->addWidget(iconLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);

    player = new GstPlayer(this);
    player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    player->setMinimumSize(100, 100);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    connect(player, &GstPlayer::faceDetected, this, &PlayerUI::showPreview);
    ControlPanel *panel = new ControlPanel(this);
    connect(panel, &ControlPanel::startPlayback, this, &PlayerUI::startPlayback);
    connect(panel, &ControlPanel::stopPlayback, this, &PlayerUI::stopPlayback);
    connect(panel, &ControlPanel::exitApplication, this, &PlayerUI::exitApplication);

    mainLayout->addWidget(headerWidget, 0);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(player, 1);
    mainLayout->addWidget(panel, 0);

    setLayout(mainLayout);
    setMinimumSize(1280, 720);
    preview = new Preview(this);
    preview->hide();
}

void PlayerUI::startPlayback()
{
    if (!player->startPlayback()) {
        qCritical() << "Playback start failed";
        exitApplication();
    }
    player->show();
}

void PlayerUI::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void PlayerUI::showPreview(QImage image, cv::Mat m)
{
    preview->setFaceImage(image, m);
    if (!preview->isVisible()) {
        QDateTime currentTime = QDateTime::currentDateTime();
        if (lastPreviewClosedTime.isValid() && currentTime.secsTo(lastPreviewClosedTime) > -2) {
            return;
        }

        QPoint topLeft = player->mapToGlobal(QPoint(0, 0));
        preview->move(topLeft.x(), topLeft.y());
        preview->show();

        connect(preview, &Preview::nameChanged, this, [this](const QString &name, cv::Mat m) {
            player->setFaceName(name, m);
        });

        connect(preview, &Preview::closed, this, [this]() {
            lastPreviewClosedTime = QDateTime::currentDateTime();
        });
    }
}

void PlayerUI::stopPlayback()
{
    player->stopPlayback();
}

void PlayerUI::exitApplication()
{
    player->stopPlayback();
    QApplication::exit();
}
