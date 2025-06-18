// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PLAYERUI_H
#define PLAYERUI_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFont>
#include <QPalette>
#include <QColor>
#include <QToolButton>
#include <QMouseEvent>
#include <QDebug>
#include <QDateTime>
#include <QImage>
#include <QMetaType>

#include "gstplayer.h"
#include "preview.h"
#include "controlpanel.h"

class Preview;
class ControlPanel;
class GstPlayer;
class PlayerUI: public QWidget {
    Q_OBJECT
    public:
        PlayerUI(QWidget* parent);
        ~PlayerUI() {}

    public slots:
        void startPlayback();
        void stopPlayback();
        void exitApplication();
        void showPreview(QImage image, cv::Mat m);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        GstPlayer* player;
        Preview *preview;
        QDateTime lastPreviewClosedTime;
};
#endif // PLAYERUI_H
