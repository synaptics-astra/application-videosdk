// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PLAYER_UI_H
#define PLAYER_UI_H

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
#include <QGuiApplication>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

#include "gstplayer.h"
#include "settingsdialog.h"
#include "controlpanel.h"
#include "command.h"

class SettingsDialog;
class ControlPanel;
class GstPlayer;
class PlayerUI: public QWidget {
    Q_OBJECT
    public:
        PlayerUI(QWidget* parent);
        ~PlayerUI();
        void setFullScreen();

    public slots:
        void startPlayback();
        void stopPlayback();
        void exitApplication();
        void showSettingsDialog();

    public:
        GstPlayer* player;
        SettingsPopup *popup;

    protected:
        void resizeEvent(QResizeEvent* event);

    private slots:
        void onSettingsChanged(const QString &inputType,
                           const QString &inputSource,
                           const QString &upscalingType,
                           const QString &modelType);

};
#endif // PLAYER_UI_H
