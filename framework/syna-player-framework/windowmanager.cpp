// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "windowmanager.h"
#include "gstplayer.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMap>
#include <QString>
#include <QSet>
#include <QDebug>
#include "settingsmanager.h"

int WindowManager::findScaleFactor(int width, int height)
{
    if (height == 0) {
        return width;
    }
    return findScaleFactor(height, width % height);
}

static int detectCameraFormat(v4l2dev_t *pV4l2Cam)
{
    struct v4l2_fmtdesc fmt;
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    QStringList formatPriority = SettingsManager::loadFormatPriority(SettingsManager::CONFIG_FILE_PATH);
    QMap<QString, uint32_t> supportedFormats;

    while (ioctl(pV4l2Cam->dev_fd, VIDIOC_ENUM_FMT, &fmt) == 0)
    {
        if (fmt.pixelformat == V4L2_PIX_FMT_YUYV) {
            supportedFormats["YUY2"] = V4L2_PIX_FMT_YUYV;
        }
        else if (fmt.pixelformat == V4L2_PIX_FMT_NV12) {
            supportedFormats["NV12"] = V4L2_PIX_FMT_NV12;
        }
        else if (fmt.pixelformat == V4L2_PIX_FMT_H264) {
            supportedFormats["H264"] = V4L2_PIX_FMT_H264;
        }
        else if (fmt.pixelformat == V4L2_PIX_FMT_MJPEG) {
            supportedFormats["MJPEG"] = V4L2_PIX_FMT_MJPEG;
        }
        fmt.index++;
    }

    for (const QString &priorityFormat : formatPriority)
    {
        if (supportedFormats.contains(priorityFormat)) {
            pV4l2Cam->dev_v4l2_fmt = supportedFormats[priorityFormat];
            return 0;
        }
    }
    return -1;
}

void WindowManager::updateCAMSRC(QVector<QString>::iterator iter)
{
    QString replaceWord = "CAM";
    QString camSrcReplaceWord = "CAMSRC";
    QString camSrcPipeline;
    QString camSrcPipelineH264, camSrcPipelineMJPEG, camSrcPipelineYUY2, camSrcPipelineNV12;
    QString videoTestSrcPipeline = "videotestsrc ! video/x-raw, framerate=30/1,format=NV12,width=640,height=480 !"
    " textoverlay text='No camera source detected..!'";
    // Predefine pipelines for each camera format
    if(machine == SL1620_ASTRA_MACHINE) {
        camSrcPipelineH264 = "v4l2src device=CAM ! video/x-h264,framerate=15/1,format=H264,width=640,height=480 ! h264parse ! avdec_h264";
        camSrcPipelineMJPEG = "v4l2src device=CAM ! image/jpeg,framerate=15/1,format=MJPEG,width=640,height=480 ! jpegdec";
        camSrcPipelineYUY2 = "v4l2src device=CAM ! video/x-raw,framerate=15/1,format=YUY2,width=640,height=480";
        camSrcPipelineNV12 = "v4l2src device=CAM ! video/x-raw,framerate=15/1,format=NV12,width=640,height=480";
    }
    else {
        camSrcPipelineH264 = "v4l2src device=CAM ! video/x-h264,framerate=30/1,format=H264,width=640,height=480 ! h264parse ! v4l2h264dec";
        camSrcPipelineMJPEG = "v4l2src device=CAM ! image/jpeg,framerate=30/1,format=MJPEG,width=640,height=480 ! jpegdec";
        camSrcPipelineYUY2 = "v4l2src device=CAM ! video/x-raw,framerate=30/1,format=YUY2,width=640,height=480";
        camSrcPipelineNV12 = "v4l2src device=CAM ! video/x-raw,framerate=30/1,format=NV12,width=640,height=480";
    }

    if (camIter == cams.end()) {
        camIter = cams.begin();
    }
    if (camsfmtIter == camsfmt.end()) {
        camsfmtIter = camsfmt.begin();
    }

    int camSrcIndex = (*iter).indexOf(camSrcReplaceWord, 0, Qt::CaseInsensitive);
    int count = camCount;
    while (camSrcIndex != -1) {

        if (count) {
            QString currentFormat = QString::fromStdString(*camsfmtIter);

            if (currentFormat == "YUY2") {
                camSrcPipeline = camSrcPipelineYUY2;
            } else if (currentFormat == "NV12") {
                camSrcPipeline = camSrcPipelineNV12;
            } else if (currentFormat == "H264") {
                camSrcPipeline = camSrcPipelineH264;
            } else if (currentFormat == "MJPEG") {
                camSrcPipeline = camSrcPipelineMJPEG;
            } else {
                qWarning() << "Unsupported camera format: " << currentFormat;
                camSrcPipeline = videoTestSrcPipeline;
            }

            (*iter).replace(camSrcIndex, camSrcReplaceWord.length(), camSrcPipeline);

            camSrcIndex = (*iter).indexOf(camSrcReplaceWord, camSrcIndex + 1, Qt::CaseInsensitive);

            if (camsfmtIter != camsfmt.end()) {
                camsfmtIter++;
            }
            count--;

        } else {
            /* In a case where we don't find sufficient number of camera source use videotestsrc */
            (*iter).replace(camSrcIndex, camSrcReplaceWord.length(), videoTestSrcPipeline);
            camSrcIndex = (*iter).indexOf(camSrcReplaceWord, camSrcIndex + 1, Qt::CaseInsensitive);
        }
    }
    int camIndex = (*iter).indexOf(replaceWord, 0, Qt::CaseInsensitive);

    while (camIndex != -1) {
        if (camCount) {
            (*iter).replace(camIndex, replaceWord.length(), QString::fromUtf8((*camIter).c_str()));
            camIter++;
            camCount--;
        }
        camIndex = (*iter).indexOf(replaceWord, camIndex + 1, Qt::CaseInsensitive);
    }
}

void WindowManager::updateRTSPSRC(QVector<QString>::iterator iter)
{
    QString rtspSrcReplaceWord = "RTSPURL";
    QStringList rtspUrls = SettingsManager::loadRTSPUrls(SettingsManager::CONFIG_FILE_PATH);
    static int rtspIndex = 0;
    if (rtspUrls.isEmpty()) {
        return;
    }

    int rtspSrcIndex = (*iter).indexOf(rtspSrcReplaceWord, 0, Qt::CaseInsensitive);
    while (rtspSrcIndex != -1) {
        (*iter).replace(rtspSrcIndex, rtspSrcReplaceWord.length(), rtspUrls[rtspIndex]);
        rtspIndex = (rtspIndex + 1) % rtspUrls.size();
        rtspSrcIndex = (*iter).indexOf(rtspSrcReplaceWord, rtspSrcIndex + 1, Qt::CaseInsensitive);
    }
}

void WindowManager::updateHRXSRC(QVector<QString>::iterator iter)
{
    QString replaceWord = "HRX";
    QString formatReplaceWord = "FMT";
    QString hrxSrcReplaceWord = "HRXSRC";
    QString hrxSrcPipeline;

    if (hrxFormatList == "YUY2" || hrxFormatList == "NV12") {
        hrxSrcPipeline = "v4l2src device=HRX ! video/x-raw,framerate=30/1,format=" + hrxFormatList + ",width=3840,height=2160";
    }

    int hrxSrcIndex = (*iter).indexOf(hrxSrcReplaceWord, 0, Qt::CaseInsensitive);
    if (hrxSrcIndex != -1) {
        (*iter).replace(hrxSrcIndex, hrxSrcReplaceWord.length(), hrxSrcPipeline);
    }

    int hrxIndex = (*iter).indexOf(replaceWord, 0, Qt::CaseInsensitive);
    if (hrxIndex != -1) {
        if (hrxCount)
        {
            (*iter).replace(hrxIndex, replaceWord.length(), QString::fromUtf8((*hrxIter).c_str()));
            hrxIter++;
            hrxCount--;
        }
    }

    int formatIndex = (*iter).indexOf(formatReplaceWord, 0, Qt::CaseInsensitive);
    if (formatIndex != -1) {
        (*iter).replace(formatIndex, formatReplaceWord.length(), hrxFormatList);
    }

    return;
}

void WindowManager::getHRXDevicePath()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    FILE *fp;
    char sysName[255];
    unsigned int index;

    udev = udev_new();
    if (!udev) {
        return;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_add_match_property(enumerate, "ID_V4L_PRODUCT", "synaptics-hrx");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        struct udev_device *dev;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        const char *sysPath = udev_device_get_syspath(dev);
        if (sysPath != nullptr) {
            std::string video4linuxPath = std::string(sysPath);
            sprintf(sysName, "%s%s%s", "cat ", video4linuxPath.c_str(), "/index");

            fp = popen(sysName, "r");
            if (fp != nullptr) {
                fscanf(fp, "%u", &index);
                pclose(fp);
                if (index == 0) {
                    std::string deviceNode = udev_device_get_devnode(dev);
                    hrxs.push_back(deviceNode);

                    v4l2dev_t hrxDetails;
                    hrxDetails.dev_fd = open(deviceNode.c_str(), O_RDWR);

                    if (hrxDetails.dev_fd < 0) {
                        qWarning() << "Failed to open the HRX Device: "
                                   << QString::fromStdString(deviceNode);
                        continue;
                    }
                    hrxFormatList = "NV12";

                    ::close(hrxDetails.dev_fd);
                }
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

void WindowManager::getDevicePath()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    FILE *fp;
    char sysName[255];
    unsigned int index;

    udev = udev_new();
    if (!udev) {
        return;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_add_match_property(enumerate, "ID_BUS", "usb");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        struct udev_device *dev;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        const char *sysPath = udev_device_get_syspath(dev);
        if (sysPath != nullptr) {
            std::string video4linuxPath = std::string(sysPath);
            sprintf(sysName, "%s%s%s", "cat ", video4linuxPath.c_str(), "/index");

            fp = popen(sysName, "r");
            if (fp != nullptr) {
                fscanf(fp, "%u", &index);
                pclose(fp);
                if (index == 0) {
                    std::string deviceNode = udev_device_get_devnode(dev);
                    cams.push_back(deviceNode);

                    v4l2dev_t camDetails;
                    camDetails.dev_fd = open(deviceNode.c_str(), O_RDWR);

                    if (camDetails.dev_fd < 0) {
                        qWarning() << "Failed to open the Camera Device: "
                                   << QString::fromStdString(deviceNode);
                        continue;
                    }
                    if (detectCameraFormat(&camDetails) == 0) {
                        if (camDetails.dev_v4l2_fmt == V4L2_PIX_FMT_YUYV) {
                            camsfmt.push_back("YUY2");
                        }
                        else if (camDetails.dev_v4l2_fmt == V4L2_PIX_FMT_NV12) {
                            camsfmt.push_back("NV12");
                        }
                        else if (camDetails.dev_v4l2_fmt == V4L2_PIX_FMT_H264) {
                            camsfmt.push_back("H264");
                        }
                        else if (camDetails.dev_v4l2_fmt == V4L2_PIX_FMT_MJPEG) {
                            camsfmt.push_back("MJPEG");
                        }
                        emit cameraFormatChanged();
                    }
                    else {
                    qWarning() << "Failed to detect suitable format for device: "
                               << QString::fromStdString(deviceNode);
                    }
                    ::close(camDetails.dev_fd);
                }
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

void WindowManager::playPipelines()
{
    while (true) {
        playPipelineSem.acquire();
        if(isPipelinePlaying) {
            for (auto player = players.begin(); player != players.end();  ++player)
            {
                (*player)->start();
                QThread::sleep (1);
            }
            closeButton->show();
        } else {
            break;
        }
    }
}

WindowManager::WindowManager(QWidget *parent, string mach) : QWidget(parent), mutex(QMutex::Recursive), machine(mach)
{
    gst_init (nullptr, nullptr);

    QList<QScreen*> screens = QGuiApplication::screens();
    int selectedScreen = -1;
    for (QScreen* screen : screens) {
        QRect screenGeometry = screen->geometry();
        if (width < screenGeometry.width()) {
            width = screenGeometry.width();
            height = screenGeometry.height();
            selectedScreen++;
        }
    }

    QScreen *targetScreen = screens.at(selectedScreen);
    setGeometry(targetScreen->geometry());

    setWindowFlags(Qt::FramelessWindowHint);
    setWindowState(windowState() | Qt::WindowFullScreen);
    setMinimumSize(640, 480);

    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
    struct wl_display* display_handle =
        (struct wl_display*)native->nativeResourceForWindow("display", nullptr);
    context = gst_wl_display_handle_context_new(display_handle);

    players.clear();
    cams.clear();
    camsfmt.clear();
    getDevicePath();
    camIter = cams.begin();
    camsfmtIter = camsfmt.begin();
    camCount = cams.count();
    hrxs.clear();
    getHRXDevicePath();
    hrxIter = hrxs.begin();
    hrxCount = hrxs.count();
}

void WindowManager::play(QString url1, QString url2)
{
    GstPlayer* player = NULL;
    int x1, y1, w1, h1;

    isPipelinePlaying = true;
    playPipelineThread = QThread::create([this]() {
        this->playPipelines();
    });

    playPipelineThread->start();

    width = geometry().width();
    height = geometry().height();
    height -= (height * 0.02);

    urls.clear();

    urls.push_back(url1);
    urls.push_back(url2);

    iter = urls.begin();
    camIter = cams.begin();
    camsfmtIter = camsfmt.begin();
    camCount = cams.count();
    hrxIter = hrxs.begin();
    hrxCount = hrxs.count();

    updateCAMSRC(iter);
    updateHRXSRC(iter);
    w1 = width;
    h1 = height;
    x1 = 0;
    y1 = 0;
    player = new GstPlayer (threadCount++, x1, y1, w1, h1, *iter, this);
    iter++;
    players.append(player);

    if(url2 != "")
    {
        updateCAMSRC(iter);
        updateHRXSRC(iter);
        x1 = 382;
        y1 = 212;
        w1 = width - x1;
        h1 = height - y1;
        player = new GstPlayer (threadCount++, x1, y1, w1, h1, *iter, this);
        iter++;
        players.append(player);
    }
    closeButton = new QPushButton("Close", this);
    closeButton->setGeometry(width - (height * 0.02 * 4), height, (height * 0.02 * 4), (height * 0.02));
    closeButton->setWindowFlags(Qt::WindowStaysOnTopHint);
    connect(closeButton, &QPushButton::clicked, this, &WindowManager::cleanUp);
    playPipelineSem.release();
}

void WindowManager::play(int index, QString url1, QString url2,
        QString url3, QString url4)
{
    urls.clear();
    urls.push_back(url1);
    urls.push_back(url2);
    urls.push_back(url3);
    urls.push_back(url4);

    int x1, y1, w1, h1, posX, posY;
    width = geometry().width();
    height = geometry().height();
    posX = frameGeometry().width() - geometry().width();
    posY = frameGeometry().height() - geometry().height();
    scaleFactor = findScaleFactor(width, height);

    width -= (2*width/scaleFactor);
    height -= (4*height/scaleFactor);

    w1 = width/index - 2*width/scaleFactor-20;
    h1 = height/index - 2*height/scaleFactor-20;

    iter = urls.begin();
    camIter = cams.begin();
    camsfmtIter = camsfmt.begin();
    camCount = cams.count();

    for (int i = 0; i < index; i++)
    {
        for (int j = 0; j < index; j++)
        {
            x1 = j*width/index + width/scaleFactor + 2*width/scaleFactor;
            y1 = i*height/index + width/scaleFactor;

            updateCAMSRC(iter);
            updateRTSPSRC(iter);
            GstPlayer* player = new GstPlayer(threadCount++, x1, y1, w1, h1,
                *iter, this);
            player->start();
            iter++;
            players.append(player);
        }
    }

    closeButton = new QPushButton("Close", this);
    closeButton->setGeometry(width - scaleFactor, height - scaleFactor/8, scaleFactor, scaleFactor/4);
    closeButton->setWindowFlags(Qt::WindowStaysOnTopHint);
    closeButton->show();
    connect(closeButton, &QPushButton::clicked, this, &WindowManager::cleanUp);
}

void WindowManager::cleanUp()
{
    emit exitThread();
}

void WindowManager::threadFinished()
{
    QMutexLocker locker(&mutex);
    --threadCount;
    if (threadCount == 0)
    {
        if(isPipelinePlaying)
        {
            isPipelinePlaying = false;
            playPipelineSem.release();
            playPipelineThread->quit();
            playPipelineThread->wait();
            delete playPipelineThread;
        }

        if (closeButton) {
            delete closeButton;
            closeButton = nullptr;
        }
        urls.clear();

        // delete player object in reverse order of allocation
        for (auto player = players.rbegin(); player != players.rend();  ++player)
        {
            delete *player;
        }
        players.clear();
        this->hide();
    }
}

