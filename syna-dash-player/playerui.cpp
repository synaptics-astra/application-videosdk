// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "playerui.h"
#include <QScreen>

PlayerUI::PlayerUI(QWidget* parent) : QWidget(parent), resolutionLabel(nullptr), fpsLabel(nullptr), videoInfoBox(nullptr), resolutionChangeBox(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(60);
    headerWidget->setStyleSheet("background-color: #007dc3;");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    QLabel *iconLabel = new QLabel(headerWidget);
    iconLabel->setPixmap(QPixmap(":/res/images/astra_icon.png").scaled(50, 50));

    QLabel *titleLabel = new QLabel("Syna Dash Player", headerWidget);
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

    videoInfoBox = new QWidget(this);
    videoInfoBox->setStyleSheet("background-color: #ffffff; border-radius: 4px; padding: 5px;");
    videoInfoBox->setFixedSize(150, 80);
    QVBoxLayout *videoInfoLayout = new QVBoxLayout(videoInfoBox);
    videoInfoLayout->setContentsMargins(3, 3, 3, 3);
    videoInfoLayout->setSpacing(2);

    QFont labelFont;
    labelFont.setPixelSize(12);

    QLabel *statsHeading = new QLabel("Video Stats", videoInfoBox);
    QFont headingFont = labelFont;
    headingFont.setPixelSize(14);
    headingFont.setBold(true);
    statsHeading->setFont(headingFont);
    statsHeading->setStyleSheet("color: #000000;");
    statsHeading->setAlignment(Qt::AlignCenter);

    resolutionLabel = new QLabel("Resolution: N/A", videoInfoBox);
    fpsLabel = new QLabel("FPS: N/A", videoInfoBox);
    resolutionLabel->setFont(labelFont);
    fpsLabel->setFont(labelFont);
    resolutionLabel->setStyleSheet("color: #000000;");
    fpsLabel->setStyleSheet("color: #000000;");

    videoInfoLayout->addWidget(statsHeading);
    videoInfoLayout->addWidget(resolutionLabel);
    videoInfoLayout->addWidget(fpsLabel);

    player = new GstPlayer(this);

    resolutionChangeBox = new QWidget(this);
    resolutionChangeBox->setStyleSheet("background-color: #ffffff; border-radius: 4px; padding: 5px;");
    resolutionChangeBox->setFixedSize(300, 30);
    QVBoxLayout *resolutionChangeLayout = new QVBoxLayout(resolutionChangeBox);
    resolutionChangeLayout->setContentsMargins(3, 3, 3, 3);
    resolutionChangeLayout->setSpacing(2);

    QLabel *resolutionChangeLabel = new QLabel("Resolution changed", resolutionChangeBox);
    resolutionChangeLabel->setFont(labelFont);
    resolutionChangeLabel->setStyleSheet("color: #000000;");
    resolutionChangeLabel->setAlignment(Qt::AlignCenter);
    resolutionChangeLabel->setWordWrap(true);

    resolutionChangeLayout->addWidget(resolutionChangeLabel);
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
    setStyleSheet("background-color: #000000;");

    headerWidget->raise();
    settingsButton->raise();
    resolutionChangeBox->raise();
    panel->raise();

    popup = new SettingsPopup(this);
    popup->hide();

    connect(popup, &SettingsPopup::settingsChanged, this, [this](const QString &inputType, const QString &inputSource) {
        player->command->setInputType(inputType);
        player->command->setInputSource(inputSource);
        player->command->createPipeline(player->width(), player->height());
    }, Qt::UniqueConnection);

    connect(player, &GstPlayer::videoInfoChanged, this, &PlayerUI::updateVideoInfoLabels);
    connect(player, &GstPlayer::resolutionChanged, this, &PlayerUI::showResolutionChangeBox);

    QString machine = player->command->machine;
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];
    QRect screenGeometry = targetScreen->geometry();
    move(screenGeometry.topLeft());
    setMinimumSize(qMin(1280, screenGeometry.width()), qMin(720, screenGeometry.height()));
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
    QString machine = player->command->machine;
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];
    QSize screenSize = targetScreen->size();
    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();

    int videoWidth = 0, videoHeight = 0;
    bool isSupported = player->command->isResolutionSupported(videoWidth, videoHeight);

    bool sizeFits = (videoWidth <= 0) || (videoWidth <= screenWidth && videoHeight <= screenHeight);

    if (!sizeFits) {
        QString message = QString("Selected video resolution %1x%2 is larger than the display resolution %3x%4 and is not supported.")
                            .arg(videoWidth).arg(videoHeight).arg(screenWidth).arg(screenHeight);
        ValidationPopup *validationPopup = new ValidationPopup(message, this);
        validationPopup->setAttribute(Qt::WA_DeleteOnClose);
        QPoint popupPos = QCursor::pos() + QPoint(10, 10);
        validationPopup->showPopup(popupPos);
        validationPopup->raise();
        validationPopup->activateWindow();

        player->command->setInputSource("");
        player->startPlayback();
        return;
    }

    if (player->command->getInputType() == "File Source" && !isSupported) {
        QString message = QString("We only support video resolutions 640x360, 960x540, and 1280x720.\nYour video resolution is %1x%2.")
                            .arg(videoWidth).arg(videoHeight);
        ValidationPopup *validationPopup = new ValidationPopup(message, this);
        validationPopup->setAttribute(Qt::WA_DeleteOnClose);
        QPoint popupPos = QCursor::pos() + QPoint(10, 10);
        validationPopup->showPopup(popupPos);
        validationPopup->raise();
        validationPopup->activateWindow();
        player->startPlayback();
        return;
    }

#ifdef HAVE_WAYLAND
    QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
    struct wl_surface* surface = static_cast<struct wl_surface*>(native->nativeResourceForWindow("surface", windowHandle()));
    if (!surface) {
        qCritical() << "Failed to get Wayland surface for screen:" << targetScreen->name();
        ValidationPopup *validationPopup = new ValidationPopup("Failed to initialize Wayland surface. Please check your display configuration.", this);
        validationPopup->setAttribute(Qt::WA_DeleteOnClose);
        QPoint popupPos = QCursor::pos() + QPoint(10, 10);
        validationPopup->showPopup(popupPos);
        validationPopup->raise();
        validationPopup->activateWindow();
        return;
    }
    player->init(surface);
#endif

    player->startPlayback();
    player->show();
    videoInfoBox->show();
}

void PlayerUI::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    QString machine = player->command->machine;
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];
    QSize screenSize = targetScreen->size();
    player->setGeometry(0, 0, qMin(width(), screenSize.width()), qMin(height(), screenSize.height()));
}

void PlayerUI::showSettingsDialog()
{
    popup->showPopup(QCursor::pos());
}

void PlayerUI::stopPlayback()
{
    player->stopPlayback();
    resolutionLabel->setText("Resolution: N/A");
    fpsLabel->setText("FPS: N/A");
    videoInfoBox->hide();
    resolutionChangeBox->hide();
}

void PlayerUI::setFullScreen()
{
    QString machine = player->command->machine;
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (machine == "sl1620" && screens.size() > 1) ? screens[1] : screens[0];
    QSize screenSize = targetScreen->size();
    setMinimumSize(qMin(1280, screenSize.width()), qMin(720, screenSize.height()));
    move(targetScreen->geometry().topLeft());
    showFullScreen();
    QTimer::singleShot(100, this, [this, targetScreen]() {
        QSize screenSize = targetScreen->size();
        player->setGeometry(0, 0, qMin(width(), screenSize.width()), qMin(height(), screenSize.height()));
        player->showFullScreen();
    });
}

void PlayerUI::exitApplication()
{
    player->stopPlayback();
    QApplication::exit();
}

void PlayerUI::updateVideoInfoLabels(const QString &resolution, float fps)
{
    resolutionLabel->setText(QString("Resolution: %1").arg(resolution));
    fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

void PlayerUI::showResolutionChangeBox(const QString &prevResolution, const QString &currResolution)
{
    QLabel *resolutionChangeLabel = resolutionChangeBox->findChild<QLabel*>();
    if (resolutionChangeLabel) {
        if(prevResolution == "N/A")
        {
            resolutionChangeLabel->setText(QString("Initial Resolution: %1").arg(currResolution));
        }
        else
        {
            resolutionChangeLabel->setText(QString("Resolution changed from %1 to %2").arg(prevResolution, currResolution));
        }
    }
    if (resolutionChangeBox) {
        resolutionChangeBox->show();
        QTimer::singleShot(2000, resolutionChangeBox, &QWidget::hide);
    }
}
