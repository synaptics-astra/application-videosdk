// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef WELCOME_PAGE_H
#define WELCOME_PAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class WelcomePage: public QWidget {
    Q_OBJECT
    public:
        WelcomePage(QWidget* parent)
        {
            QLabel* backgroundLabel = new QLabel(this);
            QPixmap pixmap(":/images/image.jpg");
            backgroundLabel->setPixmap(pixmap.scaled(1920, 1080, Qt::IgnoreAspectRatio));

            QVBoxLayout* layout = new QVBoxLayout(backgroundLabel);
            layout->addStretch();

            backgroundLabel->setLayout(layout);

            backgroundLabel->setGeometry(0, 0, 1920, 1080);

            backgroundLabel->show();
        }

};

#endif // WELCOME_PAGE_H
