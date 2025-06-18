// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QProcess>
#include <QObject>
#include "about.h"

class ChromiumLauncher : public QObject {
    Q_OBJECT
public:
    explicit ChromiumLauncher(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void openInChromium(const QString &url = " ") {
        AboutManager aboutManager;
        QString detectedMachine = aboutManager.getMachine();
        QString command;

#ifdef HAVE_WAYLAND
        if (detectedMachine == "sl1620") {
            command = "XDG_RUNTIME_DIR=/var/run/user/0 WAYLAND_DISPLAY=wayland-1 QT_QPA_PLATFORM=wayland CHROME_USER_DATA_DIR=/home/root/chromium_data/ /usr/bin/chromium --no-sandbox --alsa-output-device=hw:0,1 --restore-last-session " + url + " &";
        } else if (detectedMachine == "sl1640") {
            command = "XDG_RUNTIME_DIR=/var/run/user/0 WAYLAND_DISPLAY=wayland-1 QT_QPA_PLATFORM=wayland CHROME_USER_DATA_DIR=/home/root/chromium_data/ /usr/bin/chromium --no-sandbox --alsa-output-device=hw:0,8 --restore-last-session " + url + " &";
        } else if (detectedMachine == "sl1680") {
            command = "XDG_RUNTIME_DIR=/var/run/user/0 WAYLAND_DISPLAY=wayland-1 QT_QPA_PLATFORM=wayland CHROME_USER_DATA_DIR=/home/root/chromium_data/ /usr/bin/chromium --no-sandbox --alsa-output-device=hw:0,7 --restore-last-session " + url + " &";
        }
#endif

#ifdef HAVE_X11
        if (detectedMachine == "sl1620") {
            command = "DISPLAY=:0  QT_QPA_PLATFORM=xcb GST_GL_API=gles2 /usr/bin/chromium --no-sandbox --alsa-output-device=hw:0,1 --restore-last-session " + url + " &";
        } else if (detectedMachine == "sl1640") {
            command = "DISPLAY=:0  QT_QPA_PLATFORM=xcb GST_GL_API=gles2 /usr/bin/chromium --no-sandbox --alsa-output-device=hw:0,8 --restore-last-session " + url + " &";
        } else if (detectedMachine == "sl1680") {
            command = "DISPLAY=:0  QT_QPA_PLATFORM=xcb GST_GL_API=gles2 /usr/bin/chromium --no-sandbox --alsa-output-device=hw:0,7 --restore-last-session " + url + " &";
        }
#endif

        if (!QProcess::startDetached("/bin/sh", QStringList() << "-c" << command)) {
            qWarning() << "Failed to launch browser on" << detectedMachine;
        }
    }
};

class SynaexplorerLauncher : public QObject {
    Q_OBJECT
public:
    explicit SynaexplorerLauncher(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void openSynaexplorer(const QString &mach) {
        QString command = "synaexplorer --mach " + mach + " &";
        if (!QProcess::startDetached("/bin/sh", QStringList() << "-c" << command)) {
            qWarning() << "Failed to launch Synaexplorer.";
        }
    }
};

class TerminalLauncher : public QObject {
    Q_OBJECT
public:
    explicit TerminalLauncher(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void openTerminalLauncher() {

#ifdef HAVE_WAYLAND
        QString command = "weston-terminal &";
        if (!QProcess::startDetached("/bin/sh", QStringList() << "-c" << command)) {
            qWarning() << "Failed to launch Weston terminal.";
        }
#endif

#ifdef HAVE_X11
        QString command = "xfce4-terminal &";
        if (!QProcess::startDetached("/bin/sh", QStringList() << "-c" << command)) {
            qWarning() << "Failed to launch xfce4 terminal.";
        }
#endif
    }
};
