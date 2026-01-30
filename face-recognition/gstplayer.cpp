// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "gstplayer.h"

std::vector<cv::Point> GstPlayer::myPointCollection;
std::vector<std::string> GstPlayer::namePastCollection;
bool GstPlayer::playerflag = false;

GstPlayer::GstPlayer(QWidget* parent) : QWidget(parent)
{
    command = new Command();
    embedding = new Embedding();
    settingsManager = new SettingsManager();
    connect(this, &GstPlayer::pushFrame, this, &GstPlayer::updateImage);
}

GstPlayer::~GstPlayer()
{
    delete embedding;
    delete command;
    delete settingsManager;
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

void GstPlayer::changeInputType() {
    if (command->inputType == "cam") {
        command->inputType = "rtsp";
        rtspData = settingsManager->getData(2);
        command->urlemptyflag = false;
        if (!rtspData.isEmpty() && rtspData.contains("rtspUrls")) {
            rtspUrlsModel = rtspData.value("rtspUrls").toStringList();
            command->url = rtspUrlsModel[0];
            if (command->url == "") {
                command->urlemptyflag = true;
            }
        }
        if (rtspUrlsModel.isEmpty()) {
            qWarning() << "Error in settingsManager.getData or rtspUrls missing";
            rtspUrlsModel << "";
        }
    }
    else {
        command->inputType = "cam";
    }
}

void GstPlayer::changeRtspUrl(const QString &text) {
    if (text == "") {
        command->urlemptyflag = true;
        command->inputType = "rtsp";
        return;
    }
    command->url = text;
    rtspData = settingsManager->getData(2);
    QStringList urls = rtspData.value("rtspUrls").toStringList();
    if (urls.isEmpty()) {
        urls << "";
        command->urlemptyflag = true;
        command->inputType = "rtsp";
        return;
    }
    urls[0] = text;
    rtspData.insert("rtspUrls", urls);
    if (!settingsManager->setData(2, rtspData)) {
        qWarning() << "Failed to set default RTSP URLs";
    }
    command->urlemptyflag = false;
    command->inputType = "rtsp";
}

void GstPlayer::setFaceName(std::vector<QString> nameCollection, std::vector<cv::Mat> mCollection)
{
    embedding->registerFace(nameCollection, mCollection);
    nameCollection.clear();
    mCollection.clear();
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
                GArray* origin_x = synapMeta->type.recognition.tlx;
                GArray* origin_y = synapMeta->type.recognition.tly;
                GArray* size_x = synapMeta->type.recognition.brx;
                GArray* size_y = synapMeta->type.recognition.bry;
                GArray *out_collection = synapMeta->type.recognition.data;

                cv::Mat frame = cv::Mat(synapMeta->type.recognition.height, synapMeta->type.recognition.width, CV_8UC3, map.data);
                std::vector<cv::Point> currentPointCollection;
                std::vector<std::string> nameCurrentCollection;
                std::vector<QImage>* faceImageCollection = new std::vector<QImage>();
                std::vector<cv::Mat>* retrievedMatCollection = new std::vector<cv::Mat>();
                for (guint i = 0; i < out_collection->len; i++) {
                    cv::Mat m0 = cv::Mat(1, 512, CV_8UC1, g_array_index(out_collection, guint8* , i)).clone();
                    cv::Mat retrieved_mat = self->embedding->getNormVector(m0);
                    QString name = self->embedding->getMatchedName(retrieved_mat);
                    cv::Rect rect(g_array_index(origin_x, gint, i), g_array_index(origin_y, gint, i), g_array_index(size_x, gint, i), g_array_index(size_y, gint, i));
                    cv::Point mypoint(rect.x + rect.width / 2.0, rect.y + rect.height / 2.0);
                    if (name == " ") {
                        for (int j = 0; j < myPointCollection.size(); j++) {
                            double dis = cv::norm(myPointCollection[j] - mypoint);
                            double min = 10;
                            if ( dis < min) {
                                name = QString::fromStdString(namePastCollection[j]);
                                min = dis;
                            }
                        }
                    }
                    if (name.toStdString() != " ") {
                        currentPointCollection.push_back(mypoint);
                        nameCurrentCollection.push_back(name.toStdString());
                    }


                    if (name == " ") {
                        cv::Rect bb_rect(g_array_index(origin_x, gint, i), g_array_index(origin_y, gint, i), g_array_index(size_x, gint, i), g_array_index(size_y, gint, i));
                        cv::Mat cropped_face;
                        cropped_face = frame(bb_rect).clone();
                        if (cropped_face.cols != 112 || cropped_face.rows != 112) {
                            cv::resize(cropped_face, cropped_face, cv::Size(112, 112));
                        }
                        QImage faceImage = QImage((uint8_t*)cropped_face.data, 112, 112, QImage::Format_RGB888).copy();
                        faceImageCollection->push_back(QImage((uint8_t*)cropped_face.data, 112, 112, QImage::Format_RGB888).copy());
                        retrievedMatCollection->push_back(retrieved_mat);
                    }
                    g_free((gpointer)(g_array_index(out_collection, guint8*, i)));
                    cv::Rect box(g_array_index(origin_x, gint, i), g_array_index(origin_y, gint, i), g_array_index(size_x, gint, i), g_array_index(size_y, gint, i));
                    cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);

                    int baseline = 1;
                    cv::String text(name.toStdString());
                    cv::Point textPoint(g_array_index(origin_x, gint, i) + 5 , g_array_index(origin_y, gint, i) - 5);
                    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 2, &baseline);
                    cv::Point boxTopLeft(textPoint.x, textPoint.y - textSize.height - 1);
                    cv::Point boxBottomRight(textPoint.x + textSize.width, textPoint.y + 1);
                    if (name != " ") {
                        cv::rectangle(frame, boxTopLeft, boxBottomRight, cv::Scalar(0, 0, 0), cv::FILLED);
                    }
                    cv::putText(frame, text, textPoint, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
                }

                if (GstPlayer::playerflag) {
                    emit self->faceDetected(faceImageCollection, retrievedMatCollection);
                }

                myPointCollection = currentPointCollection;
                namePastCollection = nameCurrentCollection;
                g_array_free(origin_x, TRUE);
                g_array_free(origin_y, TRUE);
                g_array_free(size_x, TRUE);
                g_array_free(size_y, TRUE);
            }
        }

        QImage image = QImage(map.data, 640, 480, QImage::Format_RGB888).copy();
        emit self->pushFrame(image);

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}
