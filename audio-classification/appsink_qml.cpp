// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "appsink_qml.h"

AppSink::AppSink(QObject *parent)
    : QObject{parent}
{
    init();

    QString command = "echo > /sys/class/misc/synap/statistics/inference_time";

    QProcess process;
    process.setProgram("/bin/sh");
    process.setArguments({"-c", command});
    process.start();
    process.waitForFinished();
}

AppSink::~AppSink()
{
    stopPipeline();
}

void AppSink::init()
{
    QFile file("/home/root/demos/configs/AudioClassificationConfig.qml");
    if (file.open(QFile::ReadOnly)) {
        QString qmlContent = file.readAll();
        file.close();

        QStringList properties = qmlContent.split("property");

        for (int i = 1; i < properties.size(); i++) {
            QString property = properties[i].trimmed();
            int firstColonIndex = property.indexOf(":");
            int firstQuoteIndex = property.indexOf("\"", firstColonIndex);
            int lastQuoteIndex = property.indexOf("\"", firstQuoteIndex + 1);

            QString propertyName = property.mid(0, firstColonIndex).trimmed();
            QStringList propertyNameParts = propertyName.split(" ");
            QString actualPropertyName = propertyNameParts.last();
            QString propertyValue = property.mid(firstQuoteIndex + 1, lastQuoteIndex - firstQuoteIndex - 1);

            if (actualPropertyName == "confidenceThreshold") {
                bool ok;
                QStringList parts = propertyValue.split(":");
                QString value = parts.last().trimmed();
                this->confidenceThreshold = value.toDouble(&ok);
            } else if (actualPropertyName == "modelPath") {
                this->modelPath = propertyValue;
            } else if (actualPropertyName == "labelPath") {
                this->labelPath = propertyValue;
            } else if (actualPropertyName == "gstPipeline") {
                this->gstPipeline = propertyValue;
            }
        }
    } else {
        qCritical() << "Failed to open the config file";
    }

    gstPipeline.replace("MODEL_PATH", modelPath);
    readLabels(labelPath);
}

void AppSink::createPipeline() {
    GError* error = NULL;
    gst_init(nullptr, nullptr);

    pipeline = gst_parse_launch(gstPipeline.toUtf8().constData(), &error);
    if (pipeline) {
        appsink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "mysink"));
        if (appsink) {
            GstAppSinkCallbacks callbacks = {0};
            callbacks.new_sample = AppSink::newBuffer;
            gst_app_sink_set_callbacks(appsink, &callbacks, this, NULL);

            GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
            if (ret == GST_STATE_CHANGE_FAILURE) {
                qCritical() << "Failed to set pipeline state to PLAYING";
            }
        } else {
            qCritical() << "Failed to get appsink";
            stopPipeline();
        }
    } else {
        qCritical() << "Failed to create pipeline";
        stopPipeline();
    }
}

void AppSink::stopPipeline()
{
    if (pipeline) {
        gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline));
    }
    if (appsink) {
        gst_object_unref(GST_OBJECT(appsink));
    }
    gst_deinit();
}

void AppSink::updateTopLabelIndex (guint8 * scores, guint len, AppSink *appsink)
{
    int topIndex1 = -1;
    int topIndex2 = -1;
    float topScore1 = 0.0f;
    float topScore2 = 0.0f;
    QString label1 = "";
    QString label2 = "";
    float *data = reinterpret_cast<float*>(scores);

    Q_ASSERT(scores != nullptr);
    Q_ASSERT(len / 4 == appsink->labels.count());

    for (int i = 0; i < len / 4; ++i) {
        if (data[i] >= appsink->confidenceThreshold) {
            if (data[i] > topScore1) {
                topScore2 = topScore1;
                topIndex2 = topIndex1;
                topScore1 = data[i];
                topIndex1 = i;
            } else if (data[i] > topScore2) {
                topScore2 = data[i];
                topIndex2 = i;
            }
        }
    }

    QString datas;
    if (topScore1 > 0.0f) {
        label1 = appsink->labels[topIndex1];
        datas = QString("%1:%2").arg(label1).arg(topScore1);

        if (topScore2 > 0.0f) {
            label2 = appsink->labels[topIndex2];
            datas += QString(":%1:%2").arg(label2).arg(topScore2);
        } else {
            datas += ": : ";
        }

        emit appsink->sendString(datas);
    }
}

GstFlowReturn AppSink::newBuffer(GstAppSink *appsink, gpointer userData)
{
    QObject *obj = reinterpret_cast<QObject*>(userData);
    AppSink *self = qobject_cast<AppSink*>(obj);
    if (self) {
        GstSample *sample = gst_app_sink_pull_sample(appsink);
        if (!sample) {
            qCritical() << "Failed to pull sample";
            return GST_FLOW_ERROR;
        }

        GstBuffer *buffer = gst_sample_get_buffer(sample);
        if (!buffer) {
            qCritical() << "Failed to get buffer";
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        GstMapInfo map;
        if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            qCritical() << "Failed to map buffer";
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        updateTopLabelIndex(map.data, (guint) map.size, self);

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}

QString AppSink::getcpuload()
{
    QString command = "top -b -n 1 | grep idle | head -n 1 | awk '{print $8}'";

    QProcess process;
    process.setProgram("/bin/sh");
    process.setArguments({"-c", command});
    process.start();
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QString cpuLoad = QString::fromLocal8Bit(output).trimmed().remove("%");
    cpuLoad = QString("%1").arg(100 - cpuLoad.toInt());

    return cpuLoad;
}

QString AppSink::getgpuload()
{
    QProcess process;
    QString gpuLoad;
    process.start("cat /sys/kernel/debug/pvr/status");

    if (process.waitForFinished()) {
        gpuLoad = process.readAllStandardOutput();
        QStringList lines = gpuLoad.split("\n");
        foreach (const QString &line, lines) {
            if (line.contains("GPU Utilisation")) {
                gpuLoad = line.split(":")[1].trimmed().remove("%");
            }
        }
    } else {
        qCritical() << "Error: " << process.errorString();
    }

    return gpuLoad;
}

QString AppSink::getnpuload()
{
    QString command = "cat /sys/class/misc/synap/statistics/inference_time";

    QProcess process;
    process.setProgram("/bin/sh");
    process.setArguments({"-c", command});
    process.start();
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    int load = output.toInt();
    int npu_usage = (load * 100)/3000000;
    QString npuLoad = QString("%1").arg(npu_usage);

    command = "echo > /sys/class/misc/synap/statistics/inference_time";
    process.setProgram("/bin/sh");
    process.setArguments({"-c", command});
    process.start();
    process.waitForFinished();

    return npuLoad;
}

void AppSink::readLabels(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open file:" << file.errorString();
        return;
    }
    QString content = file.readAll();
    labels = content.split("\n");
    file.close();
}
