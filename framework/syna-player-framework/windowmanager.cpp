// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "windowmanager.h"
#include "gstplayer.h"

int WindowManager::findScaleFactor(int width, int height)
{
    if (height == 0) {
        return width;
    }
    return findScaleFactor(height, width % height);
}

void WindowManager::getDevicePath()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    FILE *fp;
    char sysName[255];
    dev_t devNum;
    unsigned int index;

    udev = udev_new();
    if (!udev) {
        return;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        struct udev_device *dev;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* Check if the device is a USB camera */
        if (strstr(path, "usb")) {
            devNum = udev_device_get_devnum(dev);
            sprintf (sysName, "%s%d%s", "cat /sys/class/video4linux/video",
                    minor(devNum), "/index");

            fp = popen(sysName, "r");
            if (fp != nullptr) {
                fscanf(fp, "%u", &index);
                pclose (fp);
                if (index == 0) {
                    cams.push_back(udev_device_get_devnode(dev));
                }
            }
        }
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

WindowManager::WindowManager() : mutex(QMutex::Recursive)
{
    gst_init (nullptr, nullptr);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    width = screenGeometry.width();
    height = screenGeometry.height();

    scaleFactor = findScaleFactor(width, height);

    width -= (4*width/scaleFactor);
    height -= (4*height/scaleFactor);

    setGeometry(QRect(0, 0, width, height));

    setAttribute(Qt::WA_NativeWindow);
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

    closeButton = new QPushButton("Close", this);
    closeButton->setGeometry(width - scaleFactor/2, height, scaleFactor, scaleFactor/4);
    closeButton->setWindowFlags(Qt::WindowStaysOnTopHint);
    closeButton->show();
    connect(closeButton, &QPushButton::clicked, this, &WindowManager::cleanUp);

    players.clear();
    cams.clear();
    getDevicePath();
    camIter = cams.begin();
    camCount = cams.count();
}

void WindowManager::play(int index, QString url1, QString url2,
        QString url3, QString url4)
{
    urls.clear();
    urls.push_back(url1);
    urls.push_back(url2);
    urls.push_back(url3);
    urls.push_back(url4);

    int x1, y1, w1, h1;
    w1 = width/index - 2*width/scaleFactor;
    h1 = height/index - 2*height/scaleFactor;
    iter = urls.begin();

    QString replaceWord = "CAM";
    for (int i = 0; i < index; i++) {
        for (int j = 0; j < index; j++) {
            x1 = j*width/index + width/scaleFactor + 2*width/scaleFactor;
            y1 = i*height/index + width/scaleFactor;

            int index = (*iter).indexOf(replaceWord, 0, Qt::CaseInsensitive);
            if (index != -1) {
                if (camCount) {
                    (*iter).replace(index, replaceWord.length(),
                            QString::fromUtf8((*camIter).c_str()));
                    camIter++;
                    camCount--;
                }
            }

            GstPlayer* player = new GstPlayer (threadCount++, x1, y1, w1, h1,
                    *iter, this);
            player->start();
            iter++;
            players.append(player);
        }
    }
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
        if (closeButton) {
            delete closeButton;
            closeButton = nullptr;
        }
        urls.clear();
        players.clear();
        this->close();
    }
}
