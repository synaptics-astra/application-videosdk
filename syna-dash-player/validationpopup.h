// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef VALIDATIONPOPUP_H
#define VALIDATIONPOPUP_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

class ValidationPopup : public QWidget {
    Q_OBJECT
public:
    ValidationPopup(const QString &errorMessage, QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::Popup);
        setStyleSheet("background-color: #ffffff; font-family: Arial;");
        setFixedSize(600, 200);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(10);

        QLabel *titleLabel = new QLabel("Invalid File", this);
        QFont titleFont;
        titleFont.setPixelSize(18);
        titleLabel->setFont(titleFont);
        titleLabel->setStyleSheet("color: black; font-weight: bold;");

        QLabel *messageLabel = new QLabel(errorMessage, this);
        QFont messageFont;
        messageFont.setPixelSize(14);
        messageLabel->setFont(messageFont);
        messageLabel->setStyleSheet("color: black;");
        messageLabel->setWordWrap(true);

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        QPushButton *okButton = new QPushButton("OK", this);
        okButton->setFont(QFont("Arial", 14));
        okButton->setFixedSize(120, 40);
        okButton->setStyleSheet(
            "QPushButton { background-color: #003366; color: white; border: none; padding: 10px; border-radius: 8px; }"
            "QPushButton:hover { background-color: #005999; }"
            "QPushButton:pressed { background-color: #003366; }");

        buttonLayout->addStretch();
        buttonLayout->addWidget(okButton);
        buttonLayout->addStretch();

        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(messageLabel);
        mainLayout->addLayout(buttonLayout);

        connect(okButton, &QPushButton::clicked, this, &ValidationPopup::hide);

        QTimer::singleShot(3000, this, &ValidationPopup::hide);
    }

public slots:
    void showPopup(const QPoint &pos) {
        move(pos);
        show();
    }
};

#endif // VALIDATIONPOPUP_H