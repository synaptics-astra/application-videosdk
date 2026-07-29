// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#include "gstplayer.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QMutexLocker>
#include <QStandardPaths>

namespace {
constexpr const char *kVertexShader =
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "}\n";

constexpr const char *kFragmentShader =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D uTexture;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = texture2D(uTexture, vTexCoord);\n"
    "}\n";
} // namespace

static QString materializeVideoFromResource()
{
    QFile resourceVideo(":/videos/tiny.mp4");
    if (!resourceVideo.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) {
        tempDir = QDir::tempPath();
    }

    QDir().mkpath(tempDir);
    const QString outputPath = QDir(tempDir).filePath("synaoverlay_test_video.mp4");

    QFile outputVideo(outputPath);
    if (!outputVideo.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return QString();
    }

    outputVideo.write(resourceVideo.readAll());
    outputVideo.close();

    return outputPath;
}

GstPlayer::GstPlayer(QWidget *parent) : QOpenGLWidget(parent)
{
    setAutoFillBackground(false);
}

GstPlayer::~GstPlayer()
{
    releaseTexture();
    cleanupPipeline();
}

bool GstPlayer::startPlayback()
{
    cleanupPipeline();
    framePending = false;
    updateQueued = false;

    const QString videoPath = materializeVideoFromResource();
    if (videoPath.isEmpty()) {
        qCritical() << "Failed to load test video from app.qrc";
        return false;
    }

    GError *error = nullptr;
    const QString pipelineDesc = QString(
        "multifilesrc location=\"%1\" ! qtdemux name=demux demux.video_0 ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 ! h264parse ! avdec_h264 ! synavideoconvertscale ! video/x-raw,format=RGB ! synaoverlay name=overlay mode=external ! queue ! appsink name=appSink sync=true max-buffers=1 drop=false")
        .arg(videoPath);
    const QByteArray pipelineBytes = pipelineDesc.toUtf8();

    pipeline = gst_parse_launch(pipelineBytes.constData(), &error);
    if (!pipeline) {
        qCritical() << "Failed to create pipeline:" << (error ? error->message : "unknown error");
        if (error) {
            g_error_free(error);
        }
        return false;
    }

    appsink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "appSink"));
    if (!appsink) {
        qCritical() << "Failed to find appSink in pipeline";
        cleanupPipeline();
        return false;
    }

    GstAppSinkCallbacks callbacks = {};
    callbacks.new_sample = GstPlayer::newBuffer;
    gst_app_sink_set_callbacks(appsink, &callbacks, this, nullptr);

    overlayElement = gst_bin_get_by_name(GST_BIN(pipeline), "overlay");
    if (!overlayElement) {
        qWarning() << "Overlay element not found in pipeline";
    }

    GstStateChangeReturn stateResult = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (stateResult == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to set pipeline to PLAYING";
        cleanupPipeline();
        return false;
    }

    return true;
}

void GstPlayer::stopPlayback()
{
    cleanupPipeline();
    framePending = false;
    updateQueued = false;
    QMutexLocker lock(&frameMutex);
    pendingFrame = QImage();
    videoFrame = QImage();
    textureDirty = true;
    releaseTexture();
    update();
}

void GstPlayer::cleanupPipeline()
{
    if (overlayElement) {
        gst_object_unref(overlayElement);
        overlayElement = nullptr;
    }

    if (appsink) {
        GstAppSinkCallbacks callbacks = {};
        gst_app_sink_set_callbacks(appsink, &callbacks, nullptr, nullptr);
        gst_object_unref(appsink);
        appsink = nullptr;
    }

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
}

void GstPlayer::releaseTexture()
{
    if (!frameTexture) {
        return;
    }

    if (context()) {
        makeCurrent();
        frameTexture.reset();
        doneCurrent();
        return;
    }

    frameTexture.reset();
}

void GstPlayer::initializeGL()
{
    initializeOpenGLFunctions();

    if (!shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader)) {
        qWarning() << "Failed to compile vertex shader:" << shaderProgram.log();
    }
    if (!shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader)) {
        qWarning() << "Failed to compile fragment shader:" << shaderProgram.log();
    }
    if (!shaderProgram.link()) {
        qWarning() << "Failed to link shader program:" << shaderProgram.log();
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void GstPlayer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GstPlayer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (videoFrame.isNull() || !shaderProgram.isLinked()) {
        return;
    }

    if (textureDirty || !frameTexture || frameTexture->width() != videoFrame.width() ||
        frameTexture->height() != videoFrame.height()) {
        const bool sizeChanged = !frameTexture || frameTexture->width() != videoFrame.width() ||
            frameTexture->height() != videoFrame.height();

        if (sizeChanged) {
            frameTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
            frameTexture->setFormat(QOpenGLTexture::RGB8_UNorm);
            frameTexture->setSize(videoFrame.width(), videoFrame.height());
            frameTexture->setMinificationFilter(QOpenGLTexture::Linear);
            frameTexture->setMagnificationFilter(QOpenGLTexture::Linear);
            frameTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
            frameTexture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt8);
        }

        frameTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, videoFrame.constBits());
        textureDirty = false;
    }

    if (!frameTexture) {
        return;
    }

    const float widgetRatio = static_cast<float>(width()) / static_cast<float>(height());
    const float frameRatio = static_cast<float>(videoFrame.width()) / static_cast<float>(videoFrame.height());

    float xExtent = 1.0f;
    float yExtent = 1.0f;
    if (widgetRatio > frameRatio) {
        xExtent = frameRatio / widgetRatio;
    } else {
        yExtent = widgetRatio / frameRatio;
    }

    const GLfloat vertices[] = {
        -xExtent, -yExtent, 0.0f, 1.0f,
         xExtent, -yExtent, 1.0f, 1.0f,
        -xExtent,  yExtent, 0.0f, 0.0f,
         xExtent,  yExtent, 1.0f, 0.0f,
    };

    shaderProgram.bind();
    frameTexture->bind(0);
    shaderProgram.setUniformValue("uTexture", 0);

    const int positionLoc = shaderProgram.attributeLocation("aPosition");
    const int texCoordLoc = shaderProgram.attributeLocation("aTexCoord");
    shaderProgram.enableAttributeArray(positionLoc);
    shaderProgram.enableAttributeArray(texCoordLoc);
    shaderProgram.setAttributeArray(positionLoc, GL_FLOAT, vertices, 2, 4 * sizeof(GLfloat));
    shaderProgram.setAttributeArray(texCoordLoc, GL_FLOAT, vertices + 2, 2, 4 * sizeof(GLfloat));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    shaderProgram.disableAttributeArray(positionLoc);
    shaderProgram.disableAttributeArray(texCoordLoc);
    frameTexture->release();
    shaderProgram.release();
}

void GstPlayer::updateFrame()
{
    QImage frame;
    {
        QMutexLocker lock(&frameMutex);
        frame.swap(pendingFrame);
    }

    if (frame.isNull()) {
        framePending = false;
        updateQueued = false;
        return;
    }

    videoFrame = frame;
    framePending = false;
    updateQueued = false;
    textureDirty = true;
    update();
}

GstFlowReturn GstPlayer::newBuffer(GstAppSink *appsink, gpointer userData)
{
    auto *self = static_cast<GstPlayer *>(userData);
    if (!self) {
        return GST_FLOW_ERROR;
    }

    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) {
        return GST_FLOW_ERROR;
    }

    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *s = caps ? gst_caps_get_structure(caps, 0) : nullptr;

    gint width = 0;
    gint height = 0;
    if (!s || !gst_structure_get_int(s, "width", &width) || !gst_structure_get_int(s, "height", &height)) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (!buffer) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    if (self->framePending.exchange(true)) {
        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    QImage frame(map.data, width, height, QImage::Format_RGB888);
    {
        QMutexLocker lock(&self->frameMutex);
        self->pendingFrame = frame.copy();
    }
    if (!self->updateQueued.exchange(true)) {
        QMetaObject::invokeMethod(self, "updateFrame", Qt::QueuedConnection);
    }

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}
