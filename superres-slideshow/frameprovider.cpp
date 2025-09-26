// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "frameprovider.h"

FrameProvider::FrameProvider(QObject *parent) : QObject(parent), QQuickImageProvider(QQuickImageProvider::Image)
{
    gst_init(nullptr, nullptr);
    leftData = new uint8_t[640 * 720 * 3];
    rightData = new uint8_t[1920 * 2160 * 3];
    init();
}

FrameProvider::~FrameProvider()
{
    delete[] leftData;
    delete[] rightData;
}

void FrameProvider::init()
{
    QFile file("/home/root/demos/configs/SlideshowConfig.qml");
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

            if (actualPropertyName == "modelPath") {
                this->modelPath = propertyValue;
            } else if (actualPropertyName == "gstPipeline") {
                this->gstPipeline = propertyValue;
            }
        }
    } else {
        qDebug() << "Failed to open the config file";
    }

    gstPipelineTemplate = gstPipeline;
    currentIndex = 0;

    normalFrameProcessed = false;
    srFrameProcessed = false;
}

void FrameProvider::startWithDefaultFiles()
{
    files.clear();
    QDir dir("/home/root/demos/default-slideshow-images");
    if (!dir.exists()) {
        qDebug() << "Directory /home/root/demos/default-slideshow-images does not exists";
        return;
    }
    QStringList nameFilters;
    nameFilters << "*.png";
    QStringList fileList = dir.entryList(nameFilters, QDir::Files, QDir::Name);
    for (const QString &fileName : fileList) {
         files << dir.absoluteFilePath(fileName);
    }
    if (files.isEmpty()) {
        qDebug() << "No PNG files found in /home/root/demos/default-slideshow-images";
    }
    currentIndex = 0;
    createPipeline();
    emit imageChanged();
}

void FrameProvider::addUserFiles(const QVariantList& fileUrls)
{
    const QString userFilesDir = "/home/root/demos/user-slideshow-images";

    for (const QVariant& urlVariant : fileUrls) {
        QUrl url = urlVariant.toUrl();
        QString filePath = url.toLocalFile();

        if (filePath.startsWith(userFilesDir)  && filePath.endsWith(".png") && !files.contains(filePath)) {
            files.append(filePath);
        }
    }
}

void FrameProvider::removeUserFiles(const QVariantList& fileUrls)
{
    const QString userFilesDir = "/home/root/demos/user-slideshow-images";

    for (const QVariant& urlVariant : fileUrls) {
         QUrl url = urlVariant.toUrl();
         QString filePath = url.toLocalFile();

         if(filePath.startsWith(userFilesDir) && files.contains(filePath)) {
            files.removeAll(filePath);
         }
    }
}

void FrameProvider::playUserFiles()
{
    const QString userFilesDir = "/home/root/demos/user-slideshow-images";

    files.clear();

    QDir dir(userFilesDir);
    dir.setNameFilters(QStringList() << "*.png");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList userFiles = dir.entryList();

    if(!userFiles.isEmpty()) {
       QStringList fullPaths;
       for(const QString& fileName : userFiles) {
          QString filePath = dir.filePath(fileName);
          if (!files.contains(filePath)) {
              fullPaths.append(filePath);
          }
       }
       files.append(fullPaths);

       currentIndex = 0;
       createPipeline();
       emit imageChanged();
    } else {
       qDebug() << "No .png files found in" << userFilesDir;
    }
}

void FrameProvider::playSingleFile(const QString &filePath)
{
    files.clear();
    if (filePath.endsWith(".png") && QFile::exists(filePath)) {
        files.append(filePath);
    } else {
        qDebug() << "Invalid or non-existent file:" << filePath;
        return;
    }
    currentIndex = 0;
    createPipeline();
    emit imageChanged();
}

void FrameProvider::clear()
{
    if (pipeline && GST_IS_ELEMENT(pipeline)) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
    if (appsink_1 && GST_IS_OBJECT(appsink_1)) {
        gst_object_unref(appsink_1);
        appsink_1 = nullptr;
    }
    if (appsink_2 && GST_IS_OBJECT(appsink_2)) {
        gst_object_unref(appsink_2);
        appsink_2 = nullptr;
    }
    files.clear();
    currentIndex = 0;
}

void FrameProvider::createPipeline()
{
    GError* error = NULL;
    callbackCount = 0;

    gstPipeline = gstPipelineTemplate;
    gstPipeline.replace("PNG_FILE", files[currentIndex]);
    gstPipeline.replace("MODEL_PATH", "/usr/share/synap/models/image_processing/super_resolution/model/sr_fast_y_uv_1280x720_3840x2160/model.synap");

    QFile file(files[currentIndex]);
    if (!file.exists()) {
        qCritical() << "File does not exist:" << files[currentIndex];
        return;
    }

    pipeline = gst_parse_launch(gstPipeline.toUtf8().constData(), &error);
    if (!pipeline) {
        qCritical() << "Failed to create pipeline: " << (error ? error->message : "Unknown error");
        if (error) {
            g_error_free(error);
        }
        return;
    }

    if (pipeline) {
        appsink_1 = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "srSink"));
        if (appsink_1) {
            GstAppSinkCallbacks callbacks = {0};
            callbacks.new_sample = FrameProvider::newBuffer;
            gst_app_sink_set_callbacks(appsink_1, &callbacks, this, NULL);
        }

        appsink_2 = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "normalSink"));
        if (appsink_2) {
            GstAppSinkCallbacks callbacks = {0};
            callbacks.new_sample = FrameProvider::newBuffer;
            gst_app_sink_set_callbacks(appsink_2, &callbacks, this, NULL);
        }

        GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                        (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
    if (msg != NULL) {
        gst_message_unref(msg);
    }

    gst_bus_set_flushing(bus, true);
    gst_object_unref(bus);

    if (pipeline) {
        gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline));
    }
    if (appsink_1) {
        gst_object_unref(GST_OBJECT(appsink_1));
    }
    if (appsink_2) {
        gst_object_unref(GST_OBJECT(appsink_2));
    }

    currentIndex = (currentIndex + 1) % files.size();
}

QImage FrameProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    if (id.contains("normalFrame")) {
        createPipeline();
    }

    QMutexLocker locker(&mutex_);
    QImage image;

    if (leftData != nullptr && rightData != nullptr) {
        if (id.contains("normalFrame")) {
            if (leftData != nullptr) {
                image = QImage(leftData, 640, 720, QImage::Format_RGB888).scaled(requestedSize);
                normalFrameProcessed = true;
            }
        } else if (id.contains("srFrame")) {
            if (rightData != nullptr) {
                image = QImage(rightData, 1920, 2160, QImage::Format_RGB888).scaled(requestedSize);
                srFrameProcessed = true;
            }
        }
    }

    if (normalFrameProcessed && srFrameProcessed) {
        normalFrameProcessed = false;
        srFrameProcessed = false;
    }

    return image;
}

void FrameProvider::extractHalfFrame(const uint8_t* originalFrame, int originalWidth, int originalHeight, uint8_t* halfFrame, bool left)
{
    int halfWidth = originalWidth / 2;
    int bytesPerRow = originalWidth * 3;

    if (left) {
        for (int y = 0; y < originalHeight; y++) {
            const uint8_t* originalRow = originalFrame + (y * bytesPerRow);
            uint8_t* halfRow = halfFrame + (y * halfWidth * 3);

            memcpy(halfRow, originalRow, halfWidth * 3);
        }
    } else {
        for (int y = 0; y < originalHeight; y++) {
            const uint8_t* originalRow = originalFrame + (y * bytesPerRow);
            uint8_t* halfRow = halfFrame + (y * halfWidth * 3);

            memcpy(halfRow, originalRow + halfWidth * 3, halfWidth * 3);
        }
    }
}

void FrameProvider::updateImage(uint8_t* data, int len)
{
    QMutexLocker locker(&mutex_);
    if (data != nullptr) {
        if (len == 3840 * 2160 * 3) {
            extractHalfFrame(data, 3840, 2160, rightData, true);
        } else {
            extractHalfFrame(data, 1280, 720, leftData, true);
        }
    }
}

GstFlowReturn FrameProvider::newBuffer(GstAppSink *appsink, gpointer user_data)
{
    FrameProvider *self = reinterpret_cast<FrameProvider*>(user_data);
    if (self) {
        GstSample *sample = gst_app_sink_pull_sample(appsink);
        if (!sample) {
            qCritical() << "Failed to pull sample";
            return GST_FLOW_ERROR;
        }

        GstBuffer *buffer = gst_sample_get_buffer(sample);
        if (!buffer) {
            qCritical() << "Failed to get buffer";
            return GST_FLOW_ERROR;
        }

        GstMapInfo map;
        if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            qCritical() << "Failed to map buffer";
            return GST_FLOW_ERROR;
        }

        self->updateImage(map.data, (guint) map.size);

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}