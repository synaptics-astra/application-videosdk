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
            setStyleSheet("background-color: #839192; font-family: Arial;");

            QVBoxLayout *mainLayout = new QVBoxLayout(this);
            mainLayout->setContentsMargins(20, 20, 20, 20);
            mainLayout->setSpacing(5);

            QPixmap originalPixmap(":/images/user.png");
            defaultUser = originalPixmap.scaled(112, 112, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QLabel *titleLabel = new QLabel("Add Faces", this);
            titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #1d1d1d; padding: 2px; text-align: center;");
            mainLayout->addWidget(titleLabel);
            std::vector<QWidget *> spacerWidgetCollection;
            std::vector<QLineEdit *> nameEditCollection;
            std::vector<QVBoxLayout*> imageLayoutCollection;
            for (int i = 0; i < 4 ; i++) {

                imageWidget = new QWidget(this);
                imagePreview = new QLabel(imageWidget);
                imagePreview->setScaledContents(true);
                imagePreview->setStyleSheet("border: 5px solid white;");
                imagePreviewCollection.push_back(imagePreview);

                QLineEdit *nameEdit = new QLineEdit(imageWidget);
                nameEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                nameEdit->setVisible(false);
                nameEdit->setStyleSheet("font-size: 10pt; color: #1d1d1d; border: 1px solid #cccccc; padding: 2px;");
                nameEdit->setPlaceholderText("Enter the name");
                nameEditCollection.push_back(nameEdit);

                QWidget *spacerWidget = new QWidget();
                spacerWidget->setFixedHeight(nameEdit->sizeHint().height());
                spacerWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                spacerWidget->show();
                spacerWidgetCollection.push_back(spacerWidget);

                QVBoxLayout* imageLayout = new QVBoxLayout(imageWidget);
                imageLayoutCollection.push_back(imageLayout);
                imageLayout->addStretch();
                imageLayout->addWidget(imagePreview);
                imageLayout->addWidget(nameEdit);
                imageLayout->addWidget(spacerWidget);
                imageLayout->addStretch();
                imageWidgetCollection.push_back(imageWidget);
                mainLayout->addWidget(imageWidget);
            }

            QWidget* buttonWidget = new QWidget(this);
            QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
            QPushButton *yesButton = new QPushButton("Yes", this);
            yesButton->setStyleSheet("background-color: #0078D7; color: #fff; border: none; padding: 5px 10px;");
            yesButton->setCursor(Qt::PointingHandCursor);

            QPushButton *noButton = new QPushButton("No", this);
            noButton->setStyleSheet("background-color: #ccc; color: #333; border: none; padding: 5px 10px;");
            noButton->setCursor(Qt::PointingHandCursor);

            buttonLayout->addWidget(yesButton);
            buttonLayout->addWidget(noButton);

            mainLayout->addWidget(buttonWidget);

            QWidget *inputWidget = new QWidget(this);
            inputWidget->hide();

            QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
            QPushButton *saveButton = new QPushButton("Ok", this);
            saveButton->setStyleSheet("background-color: #0078D7; color: #fff; border: none; padding: 5px 10px;");
            saveButton->setCursor(Qt::PointingHandCursor);

            QPushButton *cancelButton = new QPushButton("Cancel", this);
            cancelButton->setStyleSheet("background-color: #ccc; color: #333; border: none; padding: 5px 10px;");
            cancelButton->setCursor(Qt::PointingHandCursor);

            inputLayout->addWidget(saveButton);
            inputLayout->addWidget(cancelButton);

            mainLayout->addWidget(inputWidget);

            connect(yesButton, &QPushButton::clicked, this, [this, inputWidget, yesButton, noButton, buttonWidget, nameEditCollection, spacerWidgetCollection, imageLayoutCollection]() {
                    yesButton->hide();
                    noButton->hide();
                    buttonWidget->hide();

                    for (int i = 0 ; i < 4 ; i++) {
                        nameEditCollection[i]->setVisible(true);
                        spacerWidgetCollection[i]->hide();
                        imageLayoutCollection[i]->invalidate();
                        imageLayoutCollection[i]->activate();
                    }
                    this->updateGeometry();
                    inputWidget->show();
                    this->reg = true;
                    });

            connect(noButton, &QPushButton::clicked, this, [this, nameEditCollection]{
                    hide();
                    emit closed();
                    });

            connect(saveButton, &QPushButton::clicked, this, [this, inputWidget, yesButton, noButton, buttonWidget, nameEditCollection,spacerWidgetCollection,imageLayoutCollection]() {
                    std::vector<QString> nameCollect;
                    for (int i = 0; i < 4 ; i++) {
                        QString name = nameEditCollection[i]->text();
                        nameEditCollection[i]->clear();
                        nameEditCollection[i]->setVisible(false);
                        spacerWidgetCollection[i]->show();
                        imageLayoutCollection[i]->invalidate();
                        imageLayoutCollection[i]->activate();
                        if(!name.isEmpty()){
                            nameCollect.push_back(name);
                        }
                    }
                    this->updateGeometry();
                    emit nameChanged(nameCollect , currentFaceCollection);
                    nameCollect.clear();
                    currentFaceCollection.clear();
                    inputWidget->hide();
                    yesButton->show();
                    noButton->show();
                    buttonWidget->show();
                    hide();
                    this->reg = false;
                    emit closed();
                    });

            connect(cancelButton, &QPushButton::clicked, this, [this, inputWidget, yesButton, noButton, buttonWidget,nameEditCollection,spacerWidgetCollection,imageLayoutCollection]() {
                    for (int i = 0; i < 4 ; i++) {
                        nameEditCollection[i]->clear();
                        nameEditCollection[i]->setVisible(false);
                        spacerWidgetCollection[i]->show();
                        imageLayoutCollection[i]->invalidate();
                        imageLayoutCollection[i]->activate();
                    }
                    this->updateGeometry();
                    inputWidget->hide();
                    yesButton->show();
                    noButton->show();
                    buttonWidget->show();
                    hide();
                    this->reg = false;
                    emit closed();
                    });
        }

        void setFaceImage(std::vector<QImage>* image, std::vector<cv::Mat>* m) {
            if (!reg) {
                int size = 4;
                if (image->size() < 4) {
                    size = image->size();
                }
                int i = 0;
                for ( ; i < size ; i++) {
                    this->currentFaceCollection.push_back((*m)[i]);
                    imagePreviewCollection[i]->setPixmap(QPixmap::fromImage((*image)[i]));
                }
                QImage myImage = QImage(112, 112, QImage::Format_RGB32);
                for (int j = i; j < 4; j++) { imagePreviewCollection[j]->setPixmap(defaultUser);}
            }
        }

        std::vector<cv::Mat> currentFaceCollection;

    signals:
        void nameChanged(const std::vector<QString> &nameCollection, std::vector<cv::Mat> mCollection);
        void closed();

    public slots:
        void showPreview(const QPoint &pos) {
            move(pos);
            show();
        }

    private:
        QLabel *imagePreview;
        QWidget *imageWidget;
        std::vector<QWidget*> imageWidgetCollection;
        std::vector<QLabel*> imagePreviewCollection;
        QPixmap defaultUser;
        bool reg {false};
};

#endif // PREVIEW_H
