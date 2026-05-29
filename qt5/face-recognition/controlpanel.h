// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>

class ControlPanel : public QWidget {
    Q_OBJECT

    public:
        ControlPanel(QWidget *parent) : QWidget(parent) {
            setStyleSheet("background-color: #313131;");

            stopPreviewButton = new QToolButton(this);
            playButton = new QToolButton(this);
            stopButton = new QToolButton(this);
            exitButton = new QToolButton(this);
            rtspButton = new QToolButton(this);
            rtspOkButton = new QToolButton(this);

            stopPreviewButton->setFocusPolicy(Qt::StrongFocus);
            stopPreviewButton->setIcon(QIcon(":/images/preview.png"));
            stopPreviewButton->setIconSize(QSize(24, 24));
            playButton->setIcon(QIcon(":/images/play.png"));
            playButton->setIconSize(QSize(24, 24));
            stopButton->setIcon(QIcon(":/images/stop.png"));
            stopButton->setIconSize(QSize(24, 24));
            exitButton->setIcon(QIcon(":/images/exit.png"));
            exitButton->setIconSize(QSize(24, 24));
            rtspButton->setIcon(QIcon(":/images/rtsp.png"));
            rtspButton->setIconSize(QSize(24, 24));
            rtspOkButton->setIcon(QIcon(":/images/rtspok.png"));
            rtspOkButton->setIconSize(QSize(24, 24));

            bottomPanel = new QWidget(this);
            bottomPanel->setStyleSheet("background-color: #444444;");

            QVBoxLayout *mainLayout = new QVBoxLayout;
            QHBoxLayout *buttonLayout = new QHBoxLayout;

            QWidget *leftZone = new QWidget;
            QHBoxLayout *leftLayout = new QHBoxLayout(leftZone);
            leftLayout->addWidget(stopPreviewButton);
            leftLayout->setAlignment(Qt::AlignLeft);
            leftZone->setFixedWidth(300);

            QWidget *centerZone = new QWidget;
            QHBoxLayout *centerLayout = new QHBoxLayout(centerZone);
            centerLayout->addWidget(playButton);
            centerLayout->addWidget(stopButton);
            centerLayout->setAlignment(Qt::AlignCenter);

            QWidget *rightZone = new QWidget;
            QHBoxLayout *rightLayout = new QHBoxLayout(rightZone);

            QHBoxLayout *rtspInputLayout = new QHBoxLayout;
            nameEdit = new QLineEdit(this);
            nameEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            nameEdit->setStyleSheet("font-size: 10pt; color: #1d1d1d; border: 1px solid #cccccc; padding: 2px;");
            nameEdit->setPlaceholderText("Enter the rtsp url");
            rtspInputLayout->addWidget(nameEdit);
            rtspInputLayout->addWidget(rtspOkButton);

            rightLayout->addLayout(rtspInputLayout);
            rightLayout->addWidget(rtspButton);
            rightLayout->addWidget(exitButton);
            rightZone->setFixedWidth(300);

            leftLayout->setContentsMargins(0, 0, 0, 0);
            centerLayout->setContentsMargins(0, 0, 0, 0);
            rightLayout->setContentsMargins(0, 0, 0, 0);
            rtspInputLayout->setContentsMargins(0, 0, 0, 0);

            buttonLayout->addWidget(leftZone);
            buttonLayout->addWidget(centerZone, 1);
            buttonLayout->addWidget(rightZone);
            bottomPanel->setFixedHeight(48);
            bottomPanel->setLayout(buttonLayout);

            mainLayout->addWidget(bottomPanel);

            setLayout(mainLayout);

            connect(stopPreviewButton, &QPushButton::clicked, [this](){ this->stopPreviewButtonClicked();});
            connect(playButton, &QPushButton::clicked, [this](){ this->playButtonClicked(); });
            connect(stopButton, &QPushButton::clicked, [this](){ this->stopButtonClicked(); });
            connect(exitButton, &QPushButton::clicked, [this](){ this->exitButtonClicked(); });
            connect(rtspButton, &QPushButton::clicked, [this](){ this->rtspButtonClicked(); });
            connect(rtspOkButton, &QPushButton::clicked, [this](){ this->rtspOkButtonClicked(); });

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

        void rtspButtonClicked() {
            emit changeInputType();
            emit stopPlayback();
            emit startPlayback();
            playButton->hide();
            stopButton->show();
        }

        void rtspOkButtonClicked() {
            emit rtspUrlchange(nameEdit->text());
            emit stopPlayback();
            emit startPlayback();
            playButton->hide();
            stopButton->show();
        }

    signals:
        void stopPreview();
        void startPlayback();
        void stopPlayback();
        void exitApplication();
        void changeInputType();
        void rtspUrlchange(const QString &text);

    private:
        QToolButton *stopPreviewButton;
        QToolButton *playButton;
        QToolButton *stopButton;
        QToolButton *exitButton;
        QToolButton *rtspButton;
        QToolButton *rtspOkButton;
        QLineEdit *nameEdit;
        QWidget *bottomPanel;
};

#endif // CONTROLPANEL_H
