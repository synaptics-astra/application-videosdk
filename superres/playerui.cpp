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
    iconLabel->setPixmap(QPixmap(":/res/images/astra_icon.png").scaled(50, 50));

    QLabel *titleLabel = new QLabel("Super Resolution", headerWidget);
    QFont titleFont;
    titleFont.setPixelSize(24);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white;");

    headerLayout->addWidget(iconLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QToolButton *settingsButton = new QToolButton(this);
    settingsButton->setText("Input Config");
    QFont buttonFont;
    buttonFont.setPixelSize(18);
    settingsButton->setFont(buttonFont);
    settingsButton->setFixedSize(150, 50);
    settingsButton->setStyleSheet(
        "QToolButton {"
        "background-color: #007dc3;"
        "color: #ffffff;"
        "border: 1px solid #005999;"
        "border-radius: 8px;"
        "padding: 10px;"
        "}"
        "QToolButton:hover {"
        "background-color: #005999;"
        "}"
        "QToolButton:pressed {"
        "background-color: #003366;"
        "}");
    connect(settingsButton, &QToolButton::clicked, this, &PlayerUI::showSettingsDialog);

    QToolButton *toggleButton = new QToolButton(this);
    toggleButton->setCheckable(true);
    toggleButton->setChecked(true);

    toggleButton->setText("Super Resolution Enabled");
    toggleButton->setFont(buttonFont);
    toggleButton->setFixedSize(250, 50); // Increased width to fit title
    toggleButton->setStyleSheet(
        "QToolButton { background-color: #ccc; border: 1px solid #aaa; border-radius: 8px; padding: 10px; }"
        "QToolButton:checked { background-color: #4CAF50; border: 1px solid #3e8e41; }"
        "QToolButton:pressed { background-color: #3e8e41; }"
        "QToolButton:checked:pressed { background-color: #2f6d31; }"
    );
    connect(toggleButton, &QToolButton::toggled, [toggleButton](bool checked) {
        if (checked) {
            toggleButton->setText("Super Resolution Enabled");
        } else {
            toggleButton->setText("Super Resolution Disabled");
        }
    });

    buttonsLayout->addWidget(settingsButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(toggleButton);

    player = new GstPlayer(this);
    ControlPanel *panel = new ControlPanel(this);
    connect(panel, &ControlPanel::startPlayback, this, &PlayerUI::startPlayback);
    connect(panel, &ControlPanel::stopPlayback, this, &PlayerUI::stopPlayback);
    connect(panel, &ControlPanel::exitApplication, this, &PlayerUI::exitApplication);
    connect(toggleButton, &QToolButton::toggled, player, &GstPlayer::srEnable);

    mainLayout->addWidget(headerWidget);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(player, 1);
    mainLayout->addStretch();
    mainLayout->addWidget(panel);
    mainLayout->addStretch();

    setLayout(mainLayout);
    setMinimumSize(1280, 720);
    popup = new SettingsPopup(this);
    popup->hide();

    connect(popup, &SettingsPopup::settingsChanged, this, [this](const QString &inputType, const QString &inputSource, const QString &upscalingType, const QString &modelType) {
        player->command->setInputType(inputType);
        player->command->setInputSource(inputSource);
        player->command->setUpscalingType(upscalingType);
        player->command->setModelType(modelType);
        player->command->createPipeline(); // Apply settings
    }, Qt::UniqueConnection);

    // Launch in fullscreen mode
    setFullScreen();
}

PlayerUI::~PlayerUI()
{
    player->stopPlayback();
    delete popup;
    delete player;
}

void PlayerUI::startPlayback()
{
    int videoWidth = 0, videoHeight = 0;
    if (!player->command->isResolutionSupported(videoWidth, videoHeight)) {
        QString message = QString("We only support video resolutions 640x360, 960x540, and 1280x720.\nYour video resolution is %1x%2.")
                            .arg(videoWidth).arg(videoHeight);
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, "Unsupported Resolution", message, QMessageBox::Ok, this);
        QTimer::singleShot(3000, msgBox, &QMessageBox::accept);
        msgBox->exec();
        delete msgBox;
        player->startPlayback(); // Fallback to videotestsrc
        return;
    }

    QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
    struct wl_surface* surface = static_cast<struct wl_surface*>(native->nativeResourceForWindow("surface", windowHandle()));
    if (!surface) {
        qCritical() << "Failed to get Wayland surface";
        return;
    }

    player->init(surface);
    player->startPlayback();
    player->show();
}

void PlayerUI::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void PlayerUI::showSettingsDialog()
{
    popup->showPopup(QCursor::pos());
}

void PlayerUI::stopPlayback()
{
    player->stopPlayback();
}

void PlayerUI::setFullScreen()
{
    showFullScreen();
    QTimer::singleShot(100, this, [this]() {
        player->setGeometry(0, 0, width(), height());
        player->showFullScreen();
    });
}

void PlayerUI::exitApplication()
{
    player->stopPlayback();
    QApplication::exit();
}
