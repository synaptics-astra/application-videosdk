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

    welcomePage = new WelcomePage(this);
    welcomePage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    welcomePage->setMinimumSize(100, 100);
    player = new GstPlayer(this);
    player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    player->setMinimumSize(100, 100);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    connect(player, &GstPlayer::faceDetected, this, &PlayerUI::showPreview);
    ControlPanel *panel = new ControlPanel(this);
    panel->setFocusPolicy(Qt::StrongFocus);
    connect(panel, &ControlPanel::startPlayback, this, &PlayerUI::startPlayback);
    connect(panel, &ControlPanel::stopPlayback, this, &PlayerUI::stopPlayback);
    connect(panel, &ControlPanel::exitApplication, this, &PlayerUI::exitApplication);
    connect(panel, &ControlPanel::stopPreview, this, &PlayerUI::stopPreview);
    panel->activateWindow();
    panel->raise();
    preview = new Preview(this);

    QWidget *playerContain = new QWidget(this);
    QHBoxLayout* playerLayout = new QHBoxLayout(playerContain);
    stackedLayout = new QStackedLayout();
    playerLayout->addWidget(preview);
    playerLayout->addWidget(player);
    stackedLayout->addWidget(welcomePage);
    stackedLayout->addWidget(playerContain);
    stackedLayout->setCurrentIndex(0);

    mainLayout->addWidget(headerWidget, 0);
    mainLayout->addSpacing(50);
    mainLayout->addLayout(stackedLayout);
    mainLayout->addWidget(panel, 0);

    setLayout(mainLayout);
    setMinimumSize(1280, 720);
    preview->setAttribute(Qt::WA_ShowWithoutActivating, true);
    preview->hide();
}

void PlayerUI::startPlayback()
{
    stackedLayout->setCurrentIndex(1);
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

void PlayerUI::showPreview(std::vector<QImage>* image, std::vector<cv::Mat>* m)
{
    if (playerflag) {
        preview->setFaceImage(image, m);
        if (!preview->isVisible()) {
            QDateTime currentTime = QDateTime::currentDateTime();
            if (lastPreviewClosedTime.isValid() && currentTime.secsTo(lastPreviewClosedTime) > -2) {
                return;
            }
            QPoint topLeft = player->mapToGlobal(QPoint(0, 0));
            preview->move(topLeft.x() + 30, topLeft.y());
            preview->show();
            connect(preview, &Preview::nameChanged, this, [this](std::vector<QString> nameCollection, std::vector<cv::Mat> mCollection) {
                player->setFaceName(nameCollection, mCollection);
                nameCollection.clear();
                mCollection.clear();
                playerflag = false;
                player->playerflag = false;
            });
            connect(preview, &Preview::closed, this, [this]() {
                playerflag = false;
                player->playerflag = false;
                lastPreviewClosedTime = QDateTime::currentDateTime();
            });
        }
    }
}

void PlayerUI::stopPlayback()
{
    stackedLayout->setCurrentIndex(0);
    player->stopPlayback();
}

void PlayerUI::exitApplication()
{
    player->stopPlayback();
    QApplication::exit();
}

void PlayerUI::stopPreview()
{
    preview->hide();
    playerflag = !playerflag;
    player->playerflag = !player->playerflag;
}
