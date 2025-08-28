// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>

class ControlPanel : public QWidget {
    Q_OBJECT

    public:
        ControlPanel(QWidget *parent) : QWidget(parent) {
            setStyleSheet("background-color: #313131;");

            stopPreviewButton = new QToolButton(this);
            playButton = new QToolButton(this);
            stopButton = new QToolButton(this);
            exitButton = new QToolButton(this);

            stopPreviewButton->setFocusPolicy(Qt::StrongFocus);
            stopPreviewButton->setIcon(QIcon(":/images/preview.png"));
            stopPreviewButton->setIconSize(QSize(24, 24));
            playButton->setIcon(QIcon(":/images/play.png"));
            playButton->setIconSize(QSize(24, 24));
            stopButton->setIcon(QIcon(":/images/stop.png"));
            stopButton->setIconSize(QSize(24, 24));
            exitButton->setIcon(QIcon(":/images/exit.png"));
            exitButton->setIconSize(QSize(24, 24));

            bottomPanel = new QWidget(this);
            bottomPanel->setStyleSheet("background-color: #444444;");

            QVBoxLayout *mainLayout = new QVBoxLayout;
            QHBoxLayout *buttonLayout = new QHBoxLayout;

            buttonLayout->addWidget(stopPreviewButton);
            buttonLayout->addStretch();
            buttonLayout->addWidget(playButton);
            buttonLayout->addWidget(stopButton);
            buttonLayout->addStretch();
            buttonLayout->addWidget(exitButton);

            bottomPanel->setLayout(buttonLayout);

            mainLayout->addWidget(bottomPanel);

            setLayout(mainLayout);

            connect(stopPreviewButton, &QPushButton::clicked, [this](){ this->stopPreviewButtonClicked();});
            connect(playButton, &QPushButton::clicked, [this](){ this->playButtonClicked(); });
            connect(stopButton, &QPushButton::clicked, [this](){ this->stopButtonClicked(); });
            connect(exitButton, &QPushButton::clicked, [this](){ this->exitButtonClicked(); });

            stopButton->hide();
        }

    public slots:
        void stopPreviewButtonClicked() {
            emit stopPreview();
        }
        void playButtonClicked() {
            emit startPlayback();
            playButton->hide();
            stopButton->show();
        }

        void stopButtonClicked() {
            emit stopPlayback();
            playButton->show();
            stopButton->hide();
        }

        void exitButtonClicked() {
            emit exitApplication();
        }

    signals:
        void stopPreview();
        void startPlayback();
        void stopPlayback();
        void exitApplication();

    private:
        QToolButton *stopPreviewButton;
        QToolButton *playButton;
        QToolButton *stopButton;
        QToolButton *exitButton;
        QWidget *bottomPanel;
};

#endif // CONTROLPANEL_H
