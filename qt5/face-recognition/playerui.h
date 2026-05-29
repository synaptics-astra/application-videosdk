// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PLAYERUI_H
#define PLAYERUI_H

#include <QWidget>
#include <QStackedLayout>
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
#include "welcomepage.h"
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
        void changeInputType();
        void exitApplication();
        void showPreview(std::vector<QImage>* image, std::vector<cv::Mat>* m);
        void stopPreview();
        void rtspUrlchange(const QString &text);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        bool playerflag = false;
        GstPlayer* player;
        WelcomePage* welcomePage;
        QStackedLayout* stackedLayout;
        Preview *preview;
        QDateTime lastPreviewClosedTime;
};
#endif // PLAYERUI_H
