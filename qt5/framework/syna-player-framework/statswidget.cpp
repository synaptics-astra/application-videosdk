// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "statswidget.h"

StatsWidget::StatsWidget(QWidget *parent, QString appName, std::string mach)
    : QWidget(parent), machine(mach), hasCPU(true), hasGPU(true), hasNPU(machine != SL1620_ASTRA_MACHINE && machine != SL2619_ASTRA_MACHINE && machine != SL2615_ASTRA_MACHINE)
{
    this->setStyleSheet("background-color: rgb(0, 0, 0); color: white;");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(2);

    altWidget = new QWidget(this);
    altWidget->setStyleSheet("background-color: black;");

    QVBoxLayout *altLayout = new QVBoxLayout(altWidget);
    altLayout->setContentsMargins(50, 0, 0, 0);
    altLayout->setSpacing(50);

    QLabel *bannerLabel = new QLabel(appName, altWidget);
    bannerLabel->setAlignment(Qt::AlignCenter | Qt::AlignBottom);
    bannerLabel->setMinimumHeight(100);
    bannerLabel->setStyleSheet(
        "background-position: center;"
        "background-color: black;"
        "color: white;"
        "font-weight: bold;"
        "font-size: 24px;"
        "padding: 20px;"
    );
    altLayout->addWidget(bannerLabel);
    altLayout->addStretch();

    leftWidget = new QWidget(this);
    leftLayout = new QVBoxLayout(leftWidget);

    QLabel *titleLabel = new QLabel("System Load");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: white; font-weight: bold; font-size: 12px;");
    leftLayout->addWidget(titleLabel);

    if (hasCPU) {
        addStatRow(leftLayout, "CPU", cpuBar, cpuValue);
    }

    if (hasGPU) {
        addStatRow(leftLayout, "GPU", gpuBar, gpuValue);
    }

    if (hasNPU) {
        addStatRow(leftLayout, "NPU", npuBar, npuValue);
    }

    addStatRow(leftLayout, "MEM", memoryBar, memoryValue);
    leftLayout->addStretch();

    stack = new QStackedWidget(this);
    stack->addWidget(altWidget);
    stack->addWidget(leftWidget);
    stack->setCurrentIndex(0);

    mainLayout->addWidget(stack, 9);

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignTop | Qt::AlignRight);
    buttonLayout->addSpacing(20);

    closeButton = new QPushButton();
    closeButton->setIcon(QIcon(":/images/exit.png"));
    closeButton->setIconSize(QSize(36, 36));
    closeButton->setFixedSize(36, 36);
    closeButton->setFlat(true);
    closeButton->setToolTip("Exit");
    connect(closeButton, &QPushButton::clicked, this, &StatsWidget::handleClose);

    toggleStatsButton = new QPushButton();
    toggleStatsButton->setIcon(QIcon(":/images/stats_on.png"));
    toggleStatsButton->setIconSize(QSize(36, 36));
    toggleStatsButton->setFixedSize(36, 36);
    toggleStatsButton->setFlat(true);
    toggleStatsButton->setToolTip("Show Stats");

    connect(toggleStatsButton, &QPushButton::clicked, this, [=]() mutable {
        bool showingAlt = stack->currentIndex() == 0;
        stack->setCurrentIndex(showingAlt ? 1 : 0);

        toggleStatsButton->setIcon(QIcon(showingAlt ? ":/images/stats_off.png" : ":/images/stats_on.png"));
        toggleStatsButton->setToolTip(showingAlt ? "Hide Stats" : "Show Stats");
    });

    buttonLayout->addStretch();
    buttonLayout->addWidget(toggleStatsButton, 0, Qt::AlignCenter);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(closeButton, 0, Qt::AlignCenter);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout, 1);

    // ------------------ Stats Update Timer ------------------
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &StatsWidget::updateStats);
    updateTimer->start(2000);
}

StatsWidget::~StatsWidget() {}

void StatsWidget::handleClose()
{
    emit requestClose();
}

void StatsWidget::addStatRow(QVBoxLayout *layout, const QString &label, QProgressBar *&bar, QLabel *&valueLabel)
{
    QLabel *title = new QLabel(label);
    title->setStyleSheet("font-weight: bold; font-size: 12px; color: white;");
    title->setFixedWidth(50);

    bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setTextVisible(false);
    bar->setFixedHeight(12);
    bar->setStyleSheet(R"(
        QProgressBar {
            background-color: #444;
            border: 1px solid #222;
        }
        QProgressBar::chunk {
            background-color: #4caf50;
        }
    )");

    valueLabel = new QLabel("0%");
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    valueLabel->setStyleSheet("color: white; font-size: 12px;");
    valueLabel->setFixedWidth(40);

    QHBoxLayout *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(10);
    row->addWidget(title);
    row->addWidget(bar, 1);
    row->addWidget(valueLabel);

    layout->addLayout(row);
}

void StatsWidget::updateStats()
{
    int cpu, gpu = 0, npu = 0, memory;

    if (hasCPU) {
        cpu = getCpuLoad().toInt();
        cpuBar->setValue(cpu);
        cpuValue->setText(QString::number(cpu) + "%");
        setBarColor(cpuBar, cpu);
    }

    memory = getMemoryUsage().toInt();
    memoryBar->setValue(memory);
    memoryValue->setText(QString::number(memory) + "%");
    setBarColor(memoryBar, memory);

    if (hasGPU && gpuBar && gpuValue) {
        gpu = getGpuLoad().toInt();
        gpuBar->setValue(gpu);
        gpuValue->setText(QString::number(gpu) + "%");
        setBarColor(gpuBar, gpu);
    }

    if (hasNPU && npuBar && npuValue) {
        npu = getNpuLoad().toInt();
        npuBar->setValue(npu);
        npuValue->setText(QString::number(npu) + "%");
        setBarColor(npuBar, npu);
    }
}

void StatsWidget::setBarColor(QProgressBar *bar, int value)
{
    QString color;
    if (value < 30) color = "#00cc00";        // Green
    else if (value < 50) color = "#99cc00";   // Light green
    else if (value < 70) color = "#ffff00";   // Yellow
    else if (value < 90) color = "#ff9933";   // Light red/orange
    else color = "#cc0000";                   // Dark red

    QString style = QString(
        "QProgressBar::chunk { background-color: %1; } "
        "QProgressBar { background-color: #444; border: 1px solid #222; }"
    ).arg(color);
    bar->setStyleSheet(style);
}

// -------------------------- System Queries --------------------------

QString StatsWidget::getCpuLoad()
{
    QProcess process;
    process.start("/bin/sh", {"-c", "busybox top -b -n 1 | grep id | head -n 1 | awk '{print $8}'"});
    process.waitForFinished();
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed().remove("%");
    return QString::number(100 - output.toInt());
}

QString StatsWidget::getGpuLoad()
{
    QProcess process;
    QString output = "0";

    if (machine == SL2619_ASTRA_MACHINE || machine == SL2615_ASTRA_MACHINE) {
        process.start("cat", {"/sys/devices/platform/soc/f7980000.gpu/gpu_util"});
        if (process.waitForFinished()) {
            output = process.readAllStandardOutput().trimmed();
        } else {
            qWarning() << "Process failed:" << process.errorString();
        }
    }
    else
    {
        process.start("cat", {"/sys/kernel/debug/pvr/status"});
        if (process.waitForFinished()) {
            QString data = process.readAllStandardOutput();
            QStringList lines = data.split("\n");

            auto it = std::find_if(lines.begin(), lines.end(), [](const QString &line) {
                return line.contains("GPU Utilisation");
            });

            if (it != lines.end()) {
                output = it->split(":")[1].trimmed().remove("%");
            }
        }
    }

    return output;
}

QString StatsWidget::getNpuLoad()
{
    QString command = "cat /sys/class/misc/synap/statistics/inference_time";
    QProcess process;
    process.start("/bin/sh", {"-c", command});
    process.waitForFinished();

    int load = process.readAllStandardOutput().trimmed().toInt();
    int usage = (load * 100) / 3000000;
    QString result = QString::number(usage);

    QProcess reset;
    reset.start("/bin/sh", {"-c", "echo > /sys/class/misc/synap/statistics/inference_time"});
    reset.waitForFinished();

    return result;
}

QString StatsWidget::getMemoryUsage()
{
    QProcess process;
    process.start("/bin/sh", {"-c", "cat /proc/meminfo"});
    process.waitForFinished();
    QString output = process.readAllStandardOutput();

    QRegularExpression totalRegex(R"(MemTotal:\s+(\d+))");
    QRegularExpression freeRegex(R"(MemFree:\s+(\d+))");

    QRegularExpressionMatch matchTotal = totalRegex.match(output);
    QRegularExpressionMatch matchFree = freeRegex.match(output);

    int memTotal = matchTotal.hasMatch() ? matchTotal.captured(1).toInt() : 0;
    int memFree = matchFree.hasMatch() ? matchFree.captured(1).toInt() : 0;

    int usage = 0;
    if (memTotal > 0) {
        usage = ((memTotal - memFree) * 100) / memTotal;
    }

    return QString::number(usage);
}