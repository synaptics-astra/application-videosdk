// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef STATSWIDGET_H
#define STATSWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProcess>
#include <QDebug>
#include <QPalette>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QString>

#include "syna-player-common.h"

class StatsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatsWidget(QWidget *parent = nullptr, QString appName = QString("Demo Application"), std::string mach = "");
    ~StatsWidget();

signals:
    void requestClose();

private slots:
    void updateStats();
    void handleClose();

private:
    std::string machine;
    QStackedWidget *stack;
    QWidget *leftWidget;
    QWidget *altWidget;
    QVBoxLayout *leftLayout;
    QPushButton *closeButton;
    QPushButton *toggleStatsButton;

    QProgressBar *cpuBar;
    QProgressBar *gpuBar;
    QProgressBar *npuBar;
    QProgressBar *memoryBar;

    QLabel *cpuValue;
    QLabel *gpuValue;
    QLabel *npuValue;
    QLabel *memoryValue;

    QTimer *updateTimer;

    void addStatRow(QVBoxLayout *layout, const QString &label, QProgressBar *&bar, QLabel *&valueLabel);
    void setBarColor(QProgressBar *bar, int value);
    QString getCpuLoad();
    QString getGpuLoad();
    QString getNpuLoad();
    QString getMemoryUsage();
};

#endif // STATSWIDGET_H
