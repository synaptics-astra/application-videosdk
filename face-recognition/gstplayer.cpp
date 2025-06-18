// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "gstplayer.h"

GstPlayer::GstPlayer(QWidget* parent) : QWidget(parent)
{
    command = new Command();
    embedding = new Embedding();
    connect(this, &GstPlayer::pushFrame, this, &GstPlayer::updateImage);
}

GstPlayer::~GstPlayer()
{
    delete embedding;
    delete command;
}

void GstPlayer::updateImage(const QImage &image)
{
    videoFrame = image;
    update();
}

void GstPlayer::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    if (!videoFrame.isNull()) {
        QImage scaledImage = videoFrame.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPoint pos((width() - scaledImage.width()) / 2,
                (height() - scaledImage.height()) / 2);
        painter.drawImage(pos, scaledImage);
    }
}

bool GstPlayer::startPlayback()
{
    GError* error = NULL;
    QString url = command->createPipeline();
    string pipelineStr;
    pipelineStr = url.toUtf8().constData();

    pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
    if (!pipeline) {
        qCritical() << "Failed to create pipeline: " << error->message;
        g_error_free(error);
        return false;
    }

    appsink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "appSink"));
    if (appsink) {
        GstAppSinkCallbacks callbacks = {0};
        callbacks.new_sample = GstPlayer::newBuffer;
        gst_app_sink_set_callbacks(appsink, &callbacks, this, NULL);
    } else {
        qCritical() << "Failed to get appsink";
        return false;
    }
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return true;
}

void GstPlayer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void GstPlayer::stopPlayback()
{
    if (pipeline != nullptr) {
        gst_element_set_state (pipeline, GST_STATE_NULL);
    }

    if (appsink !=nullptr) {
        GstAppSinkCallbacks callbacks = {0};
        gst_app_sink_set_callbacks(appsink, &callbacks, NULL, NULL);
        gst_object_unref(appsink);
        appsink = nullptr;
    }

    if (pipeline != nullptr) {
        gst_object_unref (pipeline);
        pipeline = nullptr;
    }
}

void GstPlayer::setFaceName(QString name, cv::Mat m)
{
    embedding->registerFace(name, m);
}

GstFlowReturn GstPlayer::newBuffer(GstAppSink *appsink, gpointer userData)
{
    GstPlayer *self = reinterpret_cast<GstPlayer*>(userData);
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

        GstMeta *meta;
        GstSynapMeta *synapMeta;
        gpointer state = NULL;
        meta = gst_buffer_iterate_meta(buffer, &state);
        if (meta != NULL) {
            synapMeta = (GstSynapMeta*)meta;
            if (synapMeta->mode == GST_SYNAP_POSTPROCESSING_MODE_RECOGNITION) {
                cv::Mat frame = cv::Mat(synapMeta->type.recognition.height, synapMeta->type.recognition.width, CV_8UC3, map.data);
                cv::Mat m0 = cv::Mat(1, 512, CV_8UC1, synapMeta->type.recognition.data).clone();
                cv::Mat retrieved_mat = self->embedding->getNormVector(m0);

                QString name = self->embedding->getMatchedName(retrieved_mat);
                if (name == "Unknown") {
                    cv::Rect bb_rect(synapMeta->type.recognition.tlx, synapMeta->type.recognition.tly, synapMeta->type.recognition.brx, synapMeta->type.recognition.bry);
                    cv::Mat cropped_face;
                    cropped_face = frame(bb_rect).clone();
                    if (cropped_face.cols != 112 || cropped_face.rows != 112) {
                        cv::resize(cropped_face, cropped_face, cv::Size(112, 112));
                    }

                    QImage faceImage = QImage((uint8_t*)cropped_face.data, 112, 112, QImage::Format_RGB888).copy();
                    emit self->faceDetected(faceImage, retrieved_mat);
                }

                cv::Rect box(synapMeta->type.recognition.tlx, synapMeta->type.recognition.tly, synapMeta->type.recognition.brx, synapMeta->type.recognition.bry);
                cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);

                cv::String text(name.toStdString());
                cv::Point textPoint(synapMeta->type.recognition.tlx + 5 , synapMeta->type.recognition.tly + 25);
                cv::putText(frame, text, textPoint, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));
            }
        }

        QImage image = QImage(map.data, 640, 480, QImage::Format_RGB888).copy();
        emit self->pushFrame(image);

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}
