// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#include "overlaytestpanel.h"
#include "gstplayer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QDateTime>
#include <QFile>
#include <iostream>
#include <cstring>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

static cv::Mat loadOverlayImage(const QString& path)
{
    if (path.startsWith(":/")) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return cv::Mat();
        }

        const QByteArray imageBytes = file.readAll();
        std::vector<uchar> imageBuffer(imageBytes.begin(), imageBytes.end());
        return cv::imdecode(imageBuffer, cv::IMREAD_UNCHANGED);
    }

    return cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED);
}

OverlayTestPanel::OverlayTestPanel(GstPlayer* player, QWidget* parent)
    : QWidget(parent), gstPlayer(player), currentImageIndex(0), animationX(50), animationY(150),
      currentImageIndex2(0), animationX2(250), animationY2(50),
      currentImageIndex3(0), animationX3(550), animationY3(150)
{
    setupUI();
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &OverlayTestPanel::onAnimationTick);
    logMessage("Overlay Test Panel initialized");
}

OverlayTestPanel::~OverlayTestPanel()
{
}

void OverlayTestPanel::setupUI()
{
    setWindowTitle("Overlay Plugin Test Panel");
    setStyleSheet("background-color: #2b2b2b; color: #000000; font-size: 11px;");

    const QString groupBoxStyle =
        "QGroupBox {"
        " background-color: #2b2b2b;"
        " color: #ffffff;"
        " border: 1px solid #ffffff;"
        " border-radius: 3px;"
        " margin-top: 14px;"
        " padding-top: 14px;"
        "}"
        "QGroupBox::title {"
        " subcontrol-origin: padding;"
        " subcontrol-position: top center;"
        " left: 0px;"
        " padding: 4px 6px 0 6px;"
        " background-color: #2b2b2b;"
        " color: #ffffff;"
        " font-size: 18px;"
        " font-weight: 700;"
        "}";
    const QString labelStyle = "color: #ffffff;";

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // ===== Input Section =====
    QGroupBox* inputGroup = new QGroupBox("Overlay Configuration", this);
    inputGroup->setStyleSheet(groupBoxStyle);

    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);

    // Image path input
    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* imagePathLabel = new QLabel("Image Path:");
    imagePathLabel->setStyleSheet(labelStyle);
    pathLayout->addWidget(imagePathLabel);
    imagePathEdit = new QLineEdit();
    imagePathEdit->setPlaceholderText("/tmp/test_overlay.png");
    imagePathEdit->setStyleSheet("background-color: #3c3c3c; border: 1px solid #555555; padding: 5px; border-radius: 2px;");
    pathLayout->addWidget(imagePathEdit);
    browseBtn = new QPushButton("Browse...");
    browseBtn->setMaximumWidth(100);
    browseBtn->setStyleSheet("background-color: #0d47a1; color: white; border: none; padding: 5px; border-radius: 2px;");
    connect(browseBtn, &QPushButton::clicked, this, &OverlayTestPanel::onBrowseImage);
    pathLayout->addWidget(browseBtn);
    inputLayout->addLayout(pathLayout);

    // Coordinates input
    QHBoxLayout* coordLayout = new QHBoxLayout();
    coordLayout->setContentsMargins(0, 0, 0, 0);
    coordLayout->setSpacing(4);
    QLabel* xCoordLabel = new QLabel("X Coordinate:");
    xCoordLabel->setStyleSheet(labelStyle);
    coordLayout->addWidget(xCoordLabel);
    xCoordSpinBox = new QSpinBox();
    xCoordSpinBox->setMinimum(0);
    xCoordSpinBox->setMaximum(2000);
    xCoordSpinBox->setValue(100);
    xCoordSpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    xCoordSpinBox->setStyleSheet("background-color: #3c3c3c; border: 1px solid #555555;");
    coordLayout->addWidget(xCoordSpinBox);

    QLabel* yCoordLabel = new QLabel("Y Coordinate:");
    yCoordLabel->setStyleSheet(labelStyle);
    coordLayout->addWidget(yCoordLabel);
    yCoordSpinBox = new QSpinBox();
    yCoordSpinBox->setMinimum(0);
    yCoordSpinBox->setMaximum(2000);
    yCoordSpinBox->setValue(100);
    yCoordSpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    yCoordSpinBox->setStyleSheet("background-color: #3c3c3c; border: 1px solid #555555;");
    coordLayout->addWidget(yCoordSpinBox);

    inputLayout->addLayout(coordLayout);

    mainLayout->addWidget(inputGroup);

    // ===== Action Buttons =====
    QGroupBox* actionGroup = new QGroupBox("Actions", this);
    actionGroup->setStyleSheet(groupBoxStyle);

    QVBoxLayout* actionLayout = new QVBoxLayout(actionGroup);

    QHBoxLayout* pipelineLayout = new QHBoxLayout();

    startPipelineBtn = new QPushButton("Start Pipeline");
    startPipelineBtn->setStyleSheet("background-color: #2e7d32; color: white; border: none; padding: 8px; border-radius: 2px; font-weight: bold;");
    connect(startPipelineBtn, &QPushButton::clicked, this, &OverlayTestPanel::onStartPipeline);
    pipelineLayout->addWidget(startPipelineBtn);

    stopPipelineBtn = new QPushButton("Stop Pipeline");
    stopPipelineBtn->setStyleSheet("background-color: #d32f2f; color: white; border: none; padding: 8px; border-radius: 2px; font-weight: bold;");
    connect(stopPipelineBtn, &QPushButton::clicked, this, &OverlayTestPanel::onStopPipeline);
    pipelineLayout->addWidget(stopPipelineBtn);

    actionLayout->addLayout(pipelineLayout);

    QHBoxLayout* overlayActionsLayout = new QHBoxLayout();

    addOverlayBtn = new QPushButton("Add Overlay");
    addOverlayBtn->setStyleSheet("background-color: #2e7d32; color: white; border: none; padding: 8px; border-radius: 2px; font-weight: bold;");
    connect(addOverlayBtn, &QPushButton::clicked, this, &OverlayTestPanel::onAddOverlay);
    overlayActionsLayout->addWidget(addOverlayBtn);

    clearOverlaysBtn = new QPushButton("Clear All");
    clearOverlaysBtn->setStyleSheet("background-color: #d32f2f; color: white; border: none; padding: 8px; border-radius: 2px; font-weight: bold;");
    connect(clearOverlaysBtn, &QPushButton::clicked, this, &OverlayTestPanel::onClearOverlays);
    overlayActionsLayout->addWidget(clearOverlaysBtn);

    testPatternBtn = new QPushButton("Test Pattern");
    testPatternBtn->setStyleSheet("background-color: #f57c00; color: white; border: none; padding: 8px; border-radius: 2px; font-weight: bold;");
    connect(testPatternBtn, &QPushButton::clicked, this, &OverlayTestPanel::onTestPattern);
    overlayActionsLayout->addWidget(testPatternBtn);

    actionLayout->addLayout(overlayActionsLayout);

    mainLayout->addWidget(actionGroup);

    QGroupBox* logGroup = new QGroupBox("Test Log", this);
    logGroup->setStyleSheet(groupBoxStyle);

    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(200);
    logOutput->setStyleSheet("background-color: #1e1e1e; color: #00ff00; border: 1px solid #555555; font-family: 'Courier New'; font-size: 10px;");
    logLayout->addWidget(logOutput);
    mainLayout->addWidget(logGroup);
    mainLayout->addStretch();

    setMinimumWidth(500);
    setMinimumHeight(600);
}

void OverlayTestPanel::onAddOverlay()
{
    if (!gstPlayer) {
        logMessage("ERROR: GstPlayer not initialized");
        return;
    }

    QString imagePath = imagePathEdit->text().trimmed();
    if (imagePath.isEmpty()) {
        logMessage("ERROR: Image path is empty");
        return;
    }

    int x = xCoordSpinBox->value();
    int y = yCoordSpinBox->value();

    /* Load image from file with alpha channel preserved */
    cv::Mat image = loadOverlayImage(imagePath);
    if (image.empty()) {
        logMessage(QString("ERROR: Failed to load image from %1").arg(imagePath));
        return;
    }

    /* Convert to 4-channel (BGRA) to preserve transparency */
    if (image.channels() == 1) {
        /* Grayscale to BGRA */
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGRA);
    } else if (image.channels() == 3) {
        /* BGR to BGRA (add full opacity alpha channel) */
        cv::cvtColor(image, image, cv::COLOR_BGR2BGRA);
    }
    /* If already 4 channels (BGRA), keep as is for transparency support */

    logMessage(QString("Adding overlay from %1 (%2x%3, BGRA with alpha) at (%4, %5)")
        .arg(imagePath).arg(image.cols).arg(image.rows).arg(x).arg(y));

    /* Allocate raw image bytes. Caller keeps ownership and frees after command dispatch. */
    const guint64 imageDataSize = static_cast<guint64>(image.total() * image.channels());
    guint8 *image_bytes = (guint8*)g_malloc(imageDataSize);
    memcpy(image_bytes, image.data, imageDataSize);

    /* Get overlay element */
    GstElement* overlay = gstPlayer->getOverlayElement();
    if (!overlay) {
        logMessage("ERROR: Overlay element not available");
        g_free(image_bytes);
        return;
    }

    /* Set pointer and explicit size for bounds validation in plugin */
    g_object_set(overlay, "overlay-image-data", image_bytes, NULL);
    g_object_set(overlay, "overlay-image-size", imageDataSize, NULL);

    /* Build and set add-pointer command: "add-pointer:x:y:width:height:channels:data-size" */
    QString command = QString("add-pointer:%1:%2:%3:%4:%5:%6")
        .arg(x).arg(y).arg(image.cols).arg(image.rows).arg(image.channels())
        .arg(static_cast<qulonglong>(imageDataSize));

    g_object_set(overlay, "overlay-command", command.toStdString().c_str(), NULL);
    g_free(image_bytes);

    logMessage(QString("✓ Overlay added successfully using pointer method"));

    /* Sync with plugin's actual overlay list */
    syncOverlayList();
}

void OverlayTestPanel::onStartPipeline()
{
    if (!gstPlayer) {
        logMessage("ERROR: GstPlayer not initialized");
        return;
    }

    if (!gstPlayer->startPlayback()) {
        logMessage("ERROR: Unable to start playback pipeline");
        return;
    }

    logMessage("Playback pipeline started");
}

void OverlayTestPanel::onStopPipeline()
{
    if (!gstPlayer) {
        logMessage("ERROR: GstPlayer not initialized");
        return;
    }

    gstPlayer->stopPlayback();
    logMessage("Playback pipeline stopped");
}

void OverlayTestPanel::onClearOverlays()
{
    if (!gstPlayer) {
        logMessage("ERROR: GstPlayer not initialized");
        return;
    }

    logMessage("Clearing all overlays via overlay-command property...");

    /* Send clear command to overlay */
    g_object_set(gstPlayer->getOverlayElement(), "overlay-command", "clear", NULL);

    // Clear our UI records
    addedOverlays.clear();

    logMessage("✓ All overlays cleared");
    /* Sync with plugin's actual overlay list */
    syncOverlayList();
}

void OverlayTestPanel::onBrowseImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Select Overlay Image", "",
        "Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*)");

    if (!fileName.isEmpty()) {
        imagePathEdit->setText(fileName);
        logMessage("Image selected: " + fileName);
    }
}

void OverlayTestPanel::onTestPattern()
{
    if (!gstPlayer) {
        logMessage("ERROR: GstPlayer not initialized");
        return;
    }

    // Stop any running animation
    if (animationTimer->isActive()) {
        animationTimer->stop();
        logMessage("Stopped previous animation");
    }

    logMessage("=== Starting Test Pattern with triple sequential image overlay ===");

    // First set of images (loading)
    QVector<QString> testImages = {
        ":/images/loading/1.png",
        ":/images/loading/2.png",
        ":/images/loading/3.png",
        ":/images/loading/4.png",
        ":/images/loading/5.png"
    };
    // Second set of images (bars)
    QVector<QString> testImages3 = {
        ":/images/bars/1.png",
        ":/images/bars/2.png",
        ":/images/bars/3.png",
        ":/images/bars/4.png",
        ":/images/bars/5.png"
    };
    // Third set of images (columns)
    QVector<QString> testImages2 = {
        ":/images/columns/1.png",
        ":/images/columns/2.png",
        ":/images/columns/3.png",
        ":/images/columns/4.png",
        ":/images/columns/5.png"
    };

    /* Clear previous animation data */
    animationImages.clear();
    animationImages2.clear();
    animationImages3.clear();
    currentImageIndex = 0;
    currentImageIndex2 = 0;
    currentImageIndex3 = 0;
    animationX = 100;
    animationY = 150;
    animationX2 = 500;
    animationY2 = 50;
    animationX3 = 1000;
    animationY3 = 150;

    /* Load first set of test images (loading) */
    for (const auto& path : testImages) {
        cv::Mat img = loadOverlayImage(path);
        if (img.empty()) {
            logMessage(QString("ERROR: Failed to load test image from %1").arg(path));
            return;
        }
        /* Convert to BGRA if needed */
        if (img.channels() == 1) {
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
        } else if (img.channels() == 3) {
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
        }
        animationImages.push_back(img);
    }

    /* Load second set of test images (bars) */
    for (const auto& path : testImages2) {
        cv::Mat img = loadOverlayImage(path);
        if (img.empty()) {
            logMessage(QString("ERROR: Failed to load test image from %1").arg(path));
            return;
        }
        /* Convert to BGRA if needed */
        if (img.channels() == 1) {
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
        } else if (img.channels() == 3) {
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
        }
        animationImages2.push_back(img);
    }

    /* Load third set of test images (columns) */
    for (const auto& path : testImages3) {
        cv::Mat img = loadOverlayImage(path);
        if (img.empty()) {
            logMessage(QString("ERROR: Failed to load test image from %1").arg(path));
            return;
        }
        /* Convert to BGRA if needed */
        if (img.channels() == 1) {
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
        } else if (img.channels() == 3) {
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
        }
        animationImages3.push_back(img);
    }

    logMessage(QString("✓ Loaded %1 loading, %2 bar, and %3 column images - starting triple animation (1/4 second interval)")
        .arg(animationImages.size()).arg(animationImages2.size()).arg(animationImages3.size()));

    /* Display first images immediately */
    onAnimationTick();

    /* Start timer for 1/4-second interval */
    animationTimer->start(250);
}

void OverlayTestPanel::onAnimationTick()
{
    if ((animationImages.isEmpty() || animationImages2.isEmpty() || animationImages3.isEmpty()) || !gstPlayer) {
        return;
    }

    GstElement* overlay = gstPlayer->getOverlayElement();
    if (!overlay) {
        logMessage("ERROR: Overlay element not available");
        animationTimer->stop();
        return;
    }

    /* Clear previous overlays */
    g_object_set(overlay, "overlay-command", "clear", NULL);

    /* Display first set of images (loading) */
    cv::Mat& currentImg = animationImages[currentImageIndex];
    const guint64 imageDataSize = static_cast<guint64>(currentImg.total() * currentImg.channels());
    guint8 *image_bytes = (guint8*)g_malloc(imageDataSize);
    memcpy(image_bytes, currentImg.data, imageDataSize);
    g_object_set(overlay, "overlay-image-data", image_bytes, NULL);
    g_object_set(overlay, "overlay-image-size", imageDataSize, NULL);
    QString command = QString("add-pointer:%1:%2:%3:%4:%5:%6")
        .arg(animationX).arg(animationY).arg(currentImg.cols).arg(currentImg.rows).arg(currentImg.channels())
        .arg(static_cast<qulonglong>(imageDataSize));
    g_object_set(overlay, "overlay-command", command.toStdString().c_str(), NULL);
    g_free(image_bytes);

    /* Display second set of images (bars) */
    cv::Mat& currentImg2 = animationImages2[currentImageIndex2];
    const guint64 imageDataSize2 = static_cast<guint64>(currentImg2.total() * currentImg2.channels());
    guint8 *image_bytes2 = (guint8*)g_malloc(imageDataSize2);
    memcpy(image_bytes2, currentImg2.data, imageDataSize2);
    g_object_set(overlay, "overlay-image-data", image_bytes2, NULL);
    g_object_set(overlay, "overlay-image-size", imageDataSize2, NULL);

    QString command2 = QString("add-pointer:%1:%2:%3:%4:%5:%6")
        .arg(animationX2).arg(animationY2).arg(currentImg2.cols).arg(currentImg2.rows).arg(currentImg2.channels())
        .arg(static_cast<qulonglong>(imageDataSize2));
    g_object_set(overlay, "overlay-command", command2.toStdString().c_str(), NULL);
    g_free(image_bytes2);

    /* Display third set of images (columns) */
    cv::Mat& currentImg3 = animationImages3[currentImageIndex3];
    const guint64 imageDataSize3 = static_cast<guint64>(currentImg3.total() * currentImg3.channels());
    guint8 *image_bytes3 = (guint8*)g_malloc(imageDataSize3);
    memcpy(image_bytes3, currentImg3.data, imageDataSize3);
    g_object_set(overlay, "overlay-image-data", image_bytes3, NULL);
    g_object_set(overlay, "overlay-image-size", imageDataSize3, NULL);

    QString command3 = QString("add-pointer:%1:%2:%3:%4:%5:%6")
        .arg(animationX3).arg(animationY3).arg(currentImg3.cols).arg(currentImg3.rows).arg(currentImg3.channels())
        .arg(static_cast<qulonglong>(imageDataSize3));
    g_object_set(overlay, "overlay-command", command3.toStdString().c_str(), NULL);
    g_free(image_bytes3);

    /* Move to next images */
    currentImageIndex++;
    currentImageIndex2++;
    currentImageIndex3++;

    /* Restart loops if reached the end */
    if (currentImageIndex >= animationImages.size()) {
        currentImageIndex = 0;
    }
    if (currentImageIndex2 >= animationImages2.size()) {
        currentImageIndex2 = 0;
    }
    if (currentImageIndex3 >= animationImages3.size()) {
        currentImageIndex3 = 0;
    }
}

void OverlayTestPanel::logMessage(const QString& msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMsg = QString("[%1] %2").arg(timestamp, msg);
    logOutput->append(formattedMsg);

    // Also print to console
    std::cout << formattedMsg.toStdString() << std::endl;
}

void OverlayTestPanel::syncOverlayList()
{
    addedOverlays.clear();
}
