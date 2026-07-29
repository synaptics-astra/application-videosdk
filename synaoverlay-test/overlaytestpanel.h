// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#ifndef OVERLAYTESTPANEL_H
#define OVERLAYTESTPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <opencv2/core.hpp>

class GstPlayer;

class OverlayTestPanel : public QWidget {
    Q_OBJECT

public:
    OverlayTestPanel(GstPlayer* player, QWidget* parent = nullptr);
    ~OverlayTestPanel();

private slots:
    void onStartPipeline();
    void onStopPipeline();
    void onAddOverlay();
    void onClearOverlays();
    void onBrowseImage();
    void onTestPattern();
    void onAnimationTick();

private:
    void setupUI();
    void logMessage(const QString& msg);
    void syncOverlayList();  /* Read overlay items from plugin and update UI */

    GstPlayer* gstPlayer;

    // UI Elements
    QLineEdit* imagePathEdit;
    QSpinBox* xCoordSpinBox;
    QSpinBox* yCoordSpinBox;
    QPushButton* startPipelineBtn;
    QPushButton* stopPipelineBtn;
    QPushButton* addOverlayBtn;
    QPushButton* clearOverlaysBtn;
    QPushButton* browseBtn;
    QPushButton* testPatternBtn;
    QTextEdit* logOutput;
    QListWidget* overlayList;

    struct OverlayRecord {
        QString path;
        int x;
        int y;
    };

    QList<OverlayRecord> addedOverlays;

    // Animation state for test pattern (set 1: loading)
    QTimer* animationTimer;
    QVector<cv::Mat> animationImages;
    int currentImageIndex;
    int animationX;
    int animationY;

    // Animation state for second set (set 2: bars)
    QVector<cv::Mat> animationImages2;
    int currentImageIndex2;
    int animationX2;
    int animationY2;

    // Animation state for third set (set 3: columns)
    QVector<cv::Mat> animationImages3;
    int currentImageIndex3;
    int animationX3;
    int animationY3;
};

#endif // OVERLAYTESTPANEL_H
