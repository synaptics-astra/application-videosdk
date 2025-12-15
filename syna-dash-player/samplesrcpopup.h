// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SAMPLESRCPOPUP_H
#define SAMPLESRCPOPUP_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QUrl>
#include "validationpopup.h"

class SampleSrcPopup : public QWidget {
    Q_OBJECT
public:
    SampleSrcPopup(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::Popup);
        setStyleSheet("background-color: #ffffff; font-family: Arial;");
        setFixedSize(600, 400);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(10);

        QLabel *titleLabel = new QLabel("Select an URL", this);
        QFont titleFont;
        titleFont.setPixelSize(18);
        titleLabel->setFont(titleFont);
        titleLabel->setStyleSheet("color: black; font-weight: bold;");

        urlField = new QLineEdit(this);
        urlField->setPlaceholderText("Enter a video stream URL (e.g., http://example.com/stream.mpd)");
        urlField->setFont(QFont("Arial", 14));
        urlField->setStyleSheet("background-color: #f0f0f0; border: 1px solid #cccccc; border-radius: 4px; padding: 5px;");

        sampleList = new QListWidget(this);
        sampleList->setFont(QFont("Arial", 14));
        sampleList->setStyleSheet("border: 1px solid #666666; border-radius: 4px;");
        sampleList->addItem("Sample 1: http://nonprod.assets.tmecosys.com/video/upload/sp_cosy_video_h264_debug/v1705394012/videos/recipe/whole_grain_bread_folding_dough_01.mpd");
        sampleList->addItem("Sample 2: https://assets.tmecosys.com/video/upload/sp_cosy_full_hd_wifi/videos/recipe/vorwerk_video_de_01.mpd");

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *selectButton = new QPushButton("Select", this);
        QPushButton *cancelButton = new QPushButton("Cancel", this);
        selectButton->setFixedSize(120, 40);
        cancelButton->setFixedSize(120, 40);
        selectButton->setStyleSheet(
            "QPushButton { background-color: #003366; color: white; border: none; padding: 10px; border-radius: 8px; }"
            "QPushButton:hover { background-color: #005999; }");
        cancelButton->setStyleSheet(selectButton->styleSheet());

        buttonLayout->addStretch();
        buttonLayout->addWidget(selectButton);
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addStretch();

        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(urlField);
        mainLayout->addWidget(sampleList);
        mainLayout->addLayout(buttonLayout);

        connect(sampleList, &QListWidget::itemClicked, this, &SampleSrcPopup::onItemClicked);
        connect(selectButton, &QPushButton::clicked, this, &SampleSrcPopup::onSelectClicked);
        connect(cancelButton, &QPushButton::clicked, this, &SampleSrcPopup::hide);
    }

signals:
    void urlSelected(const QString &url);

public slots:
    void showPopup(const QPoint &pos) {
        move(pos);
        urlField->clear();
        sampleList->clearSelection();
        sampleList->setCurrentRow(-1);
        show();
    }

private slots:
    void onItemClicked(QListWidgetItem *item) {
        QString text = item->text();
        QString url = text.mid(text.indexOf(": ") + 2);
        urlField->setText(url);
    }

    void onSelectClicked() {
        QString url = urlField->text().trimmed();
        validateAndProcessUrl(url);
    }

private:
    void validateAndProcessUrl(const QString &url) {
        if (url.isEmpty()) {
            ValidationPopup *p = new ValidationPopup(
                "Please enter a valid HTTP/HTTPS URL ending with .mpd or select a sample.", this);
            p->setAttribute(Qt::WA_DeleteOnClose);
            p->showPopup(QCursor::pos());
            return;
        }

        QUrl qurl(url);
        bool valid = (qurl.scheme() == "http" || qurl.scheme() == "https")
                    && qurl.isValid()
                    && url.toLower().endsWith(".mpd");

        if (valid) {
            emit urlSelected(url);
            hide();
        } else {
            ValidationPopup *p = new ValidationPopup(
                "Invalid URL. Please enter a valid HTTP/HTTPS URL ending with .mpd.", this);
            p->setAttribute(Qt::WA_DeleteOnClose);
            p->showPopup(QCursor::pos());
        }
    }

    QLineEdit *urlField;
    QListWidget *sampleList;
};

#endif // SAMPLESRCPOPUP_H