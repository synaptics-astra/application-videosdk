// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPixmap>

#include "embedding.h"

class Preview : public QWidget
{
    Q_OBJECT
    public:
        Preview(QWidget *parent = nullptr) : QWidget(parent) {
            setWindowFlags(Qt::Popup);
            setStyleSheet("background-color: #839192; font-family: Arial;");

            QVBoxLayout *mainLayout = new QVBoxLayout(this);
            mainLayout->setContentsMargins(20, 20, 20, 20);
            mainLayout->setSpacing(20);

            QLabel *titleLabel = new QLabel("Add New Face?", this);
            titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #1d1d1d; text-align: left;");

            imagePreview = new QLabel(this);
            imagePreview->setStyleSheet("border: 5px solid white;");

            QHBoxLayout* imageLayout = new QHBoxLayout;
            imageLayout->addStretch();
            imageLayout->addWidget(imagePreview);
            imageLayout->addStretch();

            QHBoxLayout *buttonLayout = new QHBoxLayout;
            QPushButton *yesButton = new QPushButton("Yes", this);
            yesButton->setStyleSheet("background-color: #0078D7; color: #fff; border: none; padding: 5px 10px;");
            yesButton->setCursor(Qt::PointingHandCursor);

            QPushButton *noButton = new QPushButton("No", this);
            noButton->setStyleSheet("background-color: #ccc; color: #333; border: none; padding: 5px 10px;");
            noButton->setCursor(Qt::PointingHandCursor);

            buttonLayout->addWidget(yesButton);
            buttonLayout->addWidget(noButton);

            mainLayout->addWidget(titleLabel);
            mainLayout->addLayout(imageLayout);
            mainLayout->addLayout(buttonLayout);

            QWidget *inputWidget = new QWidget(this);
            inputWidget->hide();

            QVBoxLayout *inputLayout = new QVBoxLayout(inputWidget);
            inputLayout->setContentsMargins(0, 0, 0, 0);
            inputLayout->setSpacing(10);

            QLabel *inputLabel = new QLabel("Add Name:", inputWidget);
            inputLabel->setStyleSheet("font-size: 10pt; color: #1d1d1d;");

            QLineEdit *nameEdit = new QLineEdit(inputWidget);
            nameEdit->setStyleSheet("font-size: 10pt; color: #1d1d1d; border: 1px solid #cccccc; padding: 2px;");
            nameEdit->setPlaceholderText("Enter the name");

            QHBoxLayout *buttonLayout2 = new QHBoxLayout;
            QPushButton *saveButton = new QPushButton("Ok", inputWidget);
            saveButton->setStyleSheet("background-color: #0078D7; color: #fff; border: none; padding: 5px 10px;");
            saveButton->setCursor(Qt::PointingHandCursor);

            QPushButton *cancelButton = new QPushButton("Cancel", inputWidget);
            cancelButton->setStyleSheet("background-color: #ccc; color: #333; border: none; padding: 5px 10px;");
            cancelButton->setCursor(Qt::PointingHandCursor);

            buttonLayout2->addWidget(saveButton);
            buttonLayout2->addWidget(cancelButton);

            inputLayout->addWidget(inputLabel);
            inputLayout->addWidget(nameEdit);
            inputLayout->addLayout(buttonLayout2);

            mainLayout->addWidget(inputWidget);

            connect(yesButton, &QPushButton::clicked, this, [this, inputWidget, yesButton, noButton]() {
                    yesButton->hide();
                    noButton->hide();
                    inputWidget->show();
                    this->reg = true;
                    });

            connect(noButton, &QPushButton::clicked, this, &QWidget::hide);

            connect(saveButton, &QPushButton::clicked, this, [this, inputWidget, yesButton, noButton, nameEdit]() {
                    QString name = nameEdit->text();
                    if (!name.isEmpty()) {
                    emit nameChanged(name, current_face);
                    }
                    nameEdit->clear();
                    inputWidget->hide();
                    yesButton->show();
                    noButton->show();
                    hide();
                    this->reg = false;
                    emit closed();
                    });

            connect(cancelButton, &QPushButton::clicked, this, [this, inputWidget, yesButton, noButton]() {
                    inputWidget->hide();
                    yesButton->show();
                    noButton->show();
                    hide();
                    emit closed();
                    });
        }

        void setFaceImage(QImage image, cv::Mat m) {
            if (!reg) {
                this->current_face = m;
                imagePreview->setPixmap(QPixmap::fromImage(image));
            }
        }

    signals:
        void nameChanged(const QString &name, cv::Mat m);
        void closed();

    public slots:
        void showPreview(const QPoint &pos) {
            move(pos);
            show();
        }

    private:
        cv::Mat current_face;
        QLabel *imagePreview;
        bool reg {false};
};

#endif // PREVIEW_H
