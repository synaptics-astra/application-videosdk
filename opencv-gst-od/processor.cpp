// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#include "processor.h"
#include "qtdevice.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QString>
#include <cstdint>

#include <gst/video/video.h>

namespace {

typedef enum
{
    GST_SYNAP_POSTPROCESSING_MODE_NONE = 0,
    GST_SYNAP_POSTPROCESSING_MODE_CLASSIFIER = 1,
    GST_SYNAP_POSTPROCESSING_MODE_DETECTOR = 2,
    GST_SYNAP_POSTPROCESSING_MODE_RECOGNITION = 3,
} GstSynapPostProcessingMode;

typedef struct _GstSynapMeta GstSynapMeta;

typedef struct {
    GstMeta meta;
    gpointer data;
    gint width;
    gint height;
    gfloat confthreshold;
} GstClassifierMeta;

typedef struct {
    GstMeta meta;
    gpointer data;
    gint width;
    gint height;
} GstDetectorMeta;

typedef struct {
    GstMeta meta;
    GArray *data;
    GArray *tlx;
    GArray *tly;
    GArray *brx;
    GArray *bry;
    gint width;
    gint height;
} GstRecognitionMeta;

struct _GstSynapMeta {
    GstSynapPostProcessingMode mode;
    union {
        GstClassifierMeta classifier;
        GstDetectorMeta detector;
        GstRecognitionMeta recognition;
    } type;
};

struct Dim2d {
        int32_t x;
        int32_t y;
};

struct Landmark {
        int32_t x{};
        int32_t y{};
        int32_t z{};
        float visibility = -1.0f;
};

struct Mask {
        std::vector<float> _data;
        uint32_t _width {0};
        uint32_t _height {0};
};

struct Rect {
        Dim2d origin;
        Dim2d size;
};

struct DetectorResult {
        struct Item {
                int32_t class_index;
                float confidence;
                Rect bounding_box;
                std::vector<Landmark> landmarks;
                Mask mask;
        };

        bool success{};
        std::vector<Item> items;
};

} // namespace


CameraProcessor::CameraProcessor()
    : QObject(nullptr)
{
    gst_init(nullptr, nullptr);
    qtDevice = new QtDevice();
    inputSource = "USB Camera 1";
    width = "640";
    height = "480";
    qtDevices = qtDevice->getDevicePath();
    loadLabelsFromResource();
}

CameraProcessor::~CameraProcessor() {
    delete qtDevice;
    stop();
}

bool CameraProcessor::start(QString pipeline) {
    stop();
    v4l2src = getCamNode(inputSource);
    if (!v4l2src.isEmpty()) {
        int isVvcam = pipeline.indexOf("Input-device", 0, Qt::CaseInsensitive);
        if (isVvcam != -1) {
            pipeline.replace(isVvcam, QString("Input-device").length(), v4l2src);
        }
    } else {
            pipeline = "videotestsrc ! video/x-raw,framerate=30/1,format=RGB,width="+width+",height="+height+" ! textoverlay text='No camera source detected..!' !  appsink name=appSink";
    }
    if (pipeline.isEmpty()) {
            pipeline = "videotestsrc ! video/x-raw,framerate=30/1,format=RGB,width="+width+",height="+height+" ! textoverlay text='No pipeline is provided..!' !  appsink name=appSink";
    }

    QByteArray pipelineBytes = pipeline.toUtf8();
    GError* error = nullptr;
    this->pipeline = gst_parse_launch(pipelineBytes.constData(), &error);
    if (this->pipeline == nullptr) {
        if (error != nullptr) {
            qWarning() << "Failed to create pipeline:" << error->message;
            g_error_free(error);
        }
        return false;
    }

    appSink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(this->pipeline), "appSink"));
    if (appSink == nullptr) {
        qWarning() << "Failed to find appsink element named appSink";
        stop();
        return false;
    }

    gst_app_sink_set_emit_signals(appSink, FALSE);
    gst_app_sink_set_drop(appSink, TRUE);
    gst_app_sink_set_max_buffers(appSink, 1);

    GstAppSinkCallbacks callbacks = {};
    callbacks.new_sample = CameraProcessor::onNewSample;
    gst_app_sink_set_callbacks(appSink, &callbacks, this, nullptr);

    GstStateChangeReturn ret = gst_element_set_state(this->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "Failed to set pipeline to PLAYING";
        stop();
        return false;
    }

    qInfo() << "Opened camera using GStreamer pipeline";

    return true;
}

bool CameraProcessor::readFrame(QImage* outImage) {
    if (!outImage) {
        return false;
    }

    QMutexLocker locker(&frameMutex);
    if (latestFrame.isNull()) {
        return false;
    }

    *outImage = latestFrame;
    return true;
}

GstFlowReturn CameraProcessor::onNewSample(GstAppSink* appSink, gpointer userData) {
    CameraProcessor* self = reinterpret_cast<CameraProcessor*>(userData);
    if (self == nullptr) {
        return GST_FLOW_ERROR;
    }

    GstSample* sample = gst_app_sink_pull_sample(appSink);
    if (sample == nullptr) {
        return GST_FLOW_ERROR;
    }

    GstFlowReturn ret = self->processSample(sample);
    gst_sample_unref(sample);
    return ret;
}

GstFlowReturn CameraProcessor::processSample(GstSample* sample) {
    GstCaps* caps = gst_sample_get_caps(sample);
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (caps == nullptr || buffer == nullptr) {
        return GST_FLOW_OK;
    }

    GstVideoInfo info;
    if (!gst_video_info_from_caps(&info, caps)) {
        return GST_FLOW_OK;
    }

    GstVideoFrame videoFrame;
    if (!gst_video_frame_map(&videoFrame, &info, buffer, GST_MAP_READ)) {
        return GST_FLOW_OK;
    }

    const GstVideoFormat format = GST_VIDEO_INFO_FORMAT(&info);
    cv::Mat renderedFrame;
    QImage::Format qimageFormat = QImage::Format_Invalid;

    if (format == GST_VIDEO_FORMAT_RGB || format == GST_VIDEO_FORMAT_BGR) {
        cv::Mat frame3(
            GST_VIDEO_FRAME_HEIGHT(&videoFrame),
            GST_VIDEO_FRAME_WIDTH(&videoFrame),
            CV_8UC3,
            GST_VIDEO_FRAME_PLANE_DATA(&videoFrame, 0),
            GST_VIDEO_FRAME_PLANE_STRIDE(&videoFrame, 0));
        if (format == GST_VIDEO_FORMAT_RGB) {
            renderedFrame = frame3.clone();
            qimageFormat = QImage::Format_RGB888;
        } else {
            renderedFrame = frame3.clone();
            qimageFormat = QImage::Format_BGR888;
        }
    } else if (format == GST_VIDEO_FORMAT_RGBA || format == GST_VIDEO_FORMAT_BGRA) {
        cv::Mat frame4(
            GST_VIDEO_FRAME_HEIGHT(&videoFrame),
            GST_VIDEO_FRAME_WIDTH(&videoFrame),
            CV_8UC4,
            GST_VIDEO_FRAME_PLANE_DATA(&videoFrame, 0),
            GST_VIDEO_FRAME_PLANE_STRIDE(&videoFrame, 0));
        if (format == GST_VIDEO_FORMAT_RGBA) {
            cv::cvtColor(frame4, renderedFrame, cv::COLOR_RGBA2RGB);
            qimageFormat = QImage::Format_RGB888;
        } else {
            cv::cvtColor(frame4, renderedFrame, cv::COLOR_BGRA2BGR);
            qimageFormat = QImage::Format_BGR888;
        }
    } else {
        qWarning() << "Unsupported video format at appsink:" << gst_video_format_to_string(format);
        gst_video_frame_unmap(&videoFrame);
        return GST_FLOW_OK;
    }

    gpointer state = nullptr;
    for (GstMeta* meta = gst_buffer_iterate_meta(buffer, &state);
         meta != nullptr;
         meta = gst_buffer_iterate_meta(buffer, &state)) {
        const gchar* apiName = g_type_name(meta->info->api);
        if (apiName == nullptr || g_strrstr(apiName, "Synap") == nullptr) {
            continue;
        }

        GstSynapMeta* synapMeta = reinterpret_cast<GstSynapMeta*>(meta);
        if (synapMeta->mode != GST_SYNAP_POSTPROCESSING_MODE_DETECTOR ||
            synapMeta->type.detector.data == nullptr) {
            continue;
        }

        const DetectorResult* result = reinterpret_cast<const DetectorResult*>(synapMeta->type.detector.data);
        if (!result->success) {
            break;
        }

        const float sourceWidth = static_cast<float>(std::max(1, synapMeta->type.detector.width));
        const float sourceHeight = static_cast<float>(std::max(1, synapMeta->type.detector.height));
        const float scaleX = static_cast<float>(renderedFrame.cols) / sourceWidth;
        const float scaleY = static_cast<float>(renderedFrame.rows) / sourceHeight;

        const cv::Scalar drawColor =
            (qimageFormat == QImage::Format_RGB888) ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 0, 255);

        for (const auto& item : result->items) {
            const int x = static_cast<int>(item.bounding_box.origin.x * scaleX);
            const int y = static_cast<int>(item.bounding_box.origin.y * scaleY);
            const int w = static_cast<int>(item.bounding_box.size.x * scaleX);
            const int h = static_cast<int>(item.bounding_box.size.y * scaleY);
            if (w <= 1 || h <= 1) {
                continue;
            }

            cv::rectangle(renderedFrame, cv::Rect(x, y, w, h), drawColor, 2);

            const QString label = labelForClass(item.class_index);
            cv::putText(
                renderedFrame,
                label.toStdString(),
                cv::Point(x, std::max(15, y - 5)),
                cv::FONT_HERSHEY_SIMPLEX,
                0.5,
                drawColor,
                1,
                cv::LINE_8);
        }
        break;
    }

    gst_video_frame_unmap(&videoFrame);

    const QImage image(
        renderedFrame.data,
        renderedFrame.cols,
        renderedFrame.rows,
        static_cast<int>(renderedFrame.step),
        qimageFormat);

    const QImage frameCopy = image.copy();

    {
        QMutexLocker locker(&frameMutex);
        latestFrame = frameCopy;
    }

    emit frameReady(frameCopy);

    return GST_FLOW_OK;
}

void CameraProcessor::stop() {
    if (pipeline != nullptr) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
    }

    if (appSink != nullptr) {
        GstAppSinkCallbacks callbacks = {};
        gst_app_sink_set_callbacks(appSink, &callbacks, nullptr, nullptr);
        gst_object_unref(appSink);
        appSink = nullptr;
    }

    if (pipeline != nullptr) {
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
}

QString CameraProcessor::getCamNode(QString inputSource)
{
    for (const auto& row : qtDevices) {
        if (row.size() >= 2 && QString::fromStdString(row[0]) == inputSource) {
            return QString::fromStdString(row[1]);
        }
    }
    return QString();
}

void CameraProcessor::loadLabelsFromResource() {
    QFile file(":/info.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open labels resource :/info.json";
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Failed to parse :/info.json:" << parseError.errorString();
        return;
    }

    const QJsonValue labelsValue = doc.object().value("labels");
    if (!labelsValue.isArray()) {
        qWarning() << "Missing labels array in :/info.json";
        return;
    }

    classLabels.clear();
    const QJsonArray labelsArray = labelsValue.toArray();
    for (const QJsonValue& value : labelsArray) {
        if (value.isString()) {
            classLabels.append(value.toString());
        }
    }

    qInfo() << "Loaded" << classLabels.size() << "labels from :/info.json";
}

QString CameraProcessor::labelForClass(int classIndex) const {
    if (classIndex >= 0 && classIndex < classLabels.size()) {
        return classLabels.at(classIndex);
    }
    return QString("class_%1").arg(classIndex);
}

