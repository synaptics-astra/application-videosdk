// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "playerui.h"
#include <QScreen>

PlayerUI::PlayerUI(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(60);
    headerWidget->setStyleSheet("background-color: #007dc3;");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    QLabel *iconLabel = new QLabel(headerWidget);
    iconLabel->setPixmap(QPixmap(":/res/images/astra_icon.png").scaled(50, 50));

    QLabel *titleLabel = new QLabel("Syna Dash Player", headerWidget);
    QFont titleFont; titleFont.setPixelSize(24);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white;");

    headerLayout->addWidget(iconLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QToolButton *settingsButton = new QToolButton(this);
    settingsButton->setText("Input Config");
    settingsButton->setFixedSize(150, 50);
    settingsButton->setStyleSheet(
        "QToolButton {background-color: #007dc3;color: white;border: 1px solid #005999;border-radius: 8px;padding: 10px;}"
        "QToolButton:hover {background-color: #005999;}"
        "QToolButton:pressed {background-color: #003366;}");

    videoInfoBox = new QWidget(this);
    videoInfoBox->setStyleSheet("background-color: white; border-radius: 4px; padding: 5px;");
    videoInfoBox->setFixedSize(150, 80);
    QVBoxLayout *infoLayout = new QVBoxLayout(videoInfoBox);
    infoLayout->setContentsMargins(3,3,3,3);

    QLabel *stats = new QLabel("Video Stats", videoInfoBox);
    stats->setStyleSheet("font-weight: bold; font-size: 14px;");
    resolutionLabel = new QLabel("Resolution: N/A", videoInfoBox);
    fpsLabel = new QLabel("FPS: N/A", videoInfoBox);
    infoLayout->addWidget(stats);
    infoLayout->addWidget(resolutionLabel);
    infoLayout->addWidget(fpsLabel);

    player = new GstPlayer(this);

    resolutionChangeBox = new QWidget(this);
    resolutionChangeBox->setStyleSheet("background-color: white; border-radius: 4px; padding: 5px;");
    resolutionChangeBox->setFixedSize(300, 30);
    new QLabel("Resolution changed", resolutionChangeBox);
    resolutionChangeBox->hide();

    buttonsLayout->addWidget(settingsButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(resolutionChangeBox);
    buttonsLayout->addStretch();
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(videoInfoBox);

    connect(settingsButton, &QToolButton::clicked, this, &PlayerUI::showSettingsDialog);

    ControlPanel *panel = new ControlPanel(this);
    connect(panel, &ControlPanel::startPlayback, this, &PlayerUI::startPlayback);
    connect(panel, &ControlPanel::stopPlayback, this, &PlayerUI::stopPlayback);
    connect(panel, &ControlPanel::exitApplication, this, &PlayerUI::exitApplication);

    mainLayout->addWidget(headerWidget);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(player, 1);
    mainLayout->addWidget(panel);
    mainLayout->addStretch();
    setLayout(mainLayout);
    setStyleSheet("background-color: black;");

    popup = new SettingsPopup(this);
    popup->hide();
    connect(popup, &SettingsPopup::settingsChanged, this, [this](const QString& type, const QString& src){
        player->command->setInputType(type);
        player->command->setInputSource(src);
    });

    connect(player, &GstPlayer::videoInfoChanged, this, &PlayerUI::updateVideoInfoLabels);
    connect(player, &GstPlayer::resolutionChanged, this, &PlayerUI::showResolutionChangeBox);

    QScreen* s = (player->command->getMachine() == "sl1620" && QGuiApplication::screens().size() > 1)
                 ? QGuiApplication::screens()[1] : QGuiApplication::screens()[0];
    move(s->geometry().topLeft());
    showFullScreen();
}

PlayerUI::~PlayerUI()
{
    player->stopPlayback();
    delete popup;
    delete player;
}

void PlayerUI::startPlayback()
{
    player->startPlayback();
    player->show();
    videoInfoBox->show();
}

void PlayerUI::stopPlayback()
{
    player->stopPlayback();
    resolutionLabel->setText("Resolution: N/A");
    fpsLabel->setText("FPS: N/A");
    videoInfoBox->hide();
    resolutionChangeBox->hide();
}

void PlayerUI::showSettingsDialog()
{
    popup->showPopup(QCursor::pos());
}

void PlayerUI::exitApplication()
{
    player->stopPlayback();
    QApplication::exit();
}

void PlayerUI::updateVideoInfoLabels(const QString& res, float fps)
{
    resolutionLabel->setText("Resolution: " + res);
    fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

void PlayerUI::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (player)
        player->resize(size());
}

void PlayerUI::showResolutionChangeBox(const QString &prevResolution, const QString &currResolution)
{
    QLabel *resolutionChangeLabel = resolutionChangeBox->findChild<QLabel*>();
    if (resolutionChangeLabel) {
        if (prevResolution == "N/A") {
            resolutionChangeLabel->setText(QString("Initial Resolution: %1").arg(currResolution));
        } else {
            resolutionChangeLabel->setText(QString("Resolution changed from %1 to %2").arg(prevResolution, currResolution));
        }
        resolutionChangeLabel->adjustSize();           // This is the only fix needed
    }
    resolutionChangeBox->show();
    QTimer::singleShot(2000, resolutionChangeBox, &QWidget::hide);
}