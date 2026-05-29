// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#include <QApplication>
#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdlib>

#include "processor.h"

class CameraWindow : public QWidget {
public:
    CameraWindow() {
        setWindowTitle("OpenCV GStreamer Camera Preview");
        resize(960, 540);

        setStyleSheet("background-color: black;");

        QString pipeline = "v4l2src device=Input-device ! video/x-raw,framerate=30/1,format=YUY2,width=640,height=480 ! synavideoconvertscale ! video/x-raw,format=RGB,width=640,height=384 ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector output=overlay frameinterval=1 ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! appsink name=appSink drop=1 sync=false max-buffers=1";

        connect(&processor, &CameraProcessor::frameReady, this, &CameraWindow::onFrameReady, Qt::QueuedConnection);

        if (!processor.start(pipeline)) {
            return;
        }
    }

    ~CameraWindow() override {
        processor.stop();
    }

private:
    void onFrameReady(const QImage &image) {
        videoFrame = image;
        update();
    }

    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
        if (!videoFrame.isNull()) {
            const QImage scaledImage = videoFrame.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            const QPoint pos((width() - scaledImage.width()) / 2, (height() - scaledImage.height()) / 2);
            painter.drawImage(pos, scaledImage);
        }
    }

    QImage videoFrame;
    CameraProcessor processor;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    CameraWindow window;
    window.show();

    return app.exec();
}
