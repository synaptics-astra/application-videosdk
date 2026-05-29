// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Synaptics Incorporated

#include "qtdevice.h"

QVector<QVector<string>> QtDevice::getDevicePath()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    FILE *fp;
    char sysName[255];
    unsigned int index;
    int usbCameraCount = 1;

    udev = udev_new();
    if (!udev) {
        return QVector<QVector<string>>();
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
                    qtDevices.push_back(QVector<string>() <<
                            QString("USB Camera %1").arg(usbCameraCount).toStdString() << deviceNode);
                    usbCameraCount++;
                }
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return qtDevices;
}

