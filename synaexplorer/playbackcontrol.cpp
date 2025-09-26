// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "playbackcontrol.h"
#include <QProcess>

PlaybackControl::PlaybackControl(string machine) : machine(machine)
{
    windowManager = new WindowManager(nullptr, this->machine, "Synaptics Explorer");
}

PlaybackControl::~PlaybackControl()
{
    delete windowManager;
}

void PlaybackControl::doPlayback(int index, QString url1, QString url2,
        QString url3, QString url4)
{
    windowManager->show();
    windowManager->play(index, url1, url2, url3, url4);
}

void PlaybackControl::launch(QString path)
{
    QProcess* process = new QProcess();
    QStringList arguments;
    QStringList environmentVariables;

#ifdef HAVE_WAYLAND
    environmentVariables << "XDG_RUNTIME_DIR=/var/run/user/0";
    environmentVariables << "WAYLAND_DISPLAY=wayland-1";
    environmentVariables << "QT_QPA_PLATFORM=wayland";
#endif

#ifdef HAVE_X11
    environmentVariables << "DISPLAY=:0";
    environmentVariables << "QT_QPA_PLATFORM=xcb";
    environmentVariables << "GST_GL_API=gles2";
#endif

    process->setProgram(path);
    arguments << path;

    if (path == "/usr/bin/syna-video-player") {
        arguments << "--mach" << QString::fromStdString(machine);
        arguments << "--mode";
        if (machine == "sl1620") {
            arguments << "ffmpeg";
        } else {
            arguments << "v4l2";
        }
    } else if (path == "/usr/bin/syna-ai-player"
            || path == "/usr/bin/syna-capability-demo") {
        arguments << "--mach" << QString::fromStdString(machine);
    }

    process->setArguments(arguments);

    QProcessEnvironment env;
    for (const QString& envVar : environmentVariables) {
        QStringList envPair = envVar.split("=");
        if (envPair.size() == 2) {
            env.insert(envPair[0], envPair[1]);
        }
    }
    process->setProcessEnvironment(env);

    bool started = process->startDetached();
    if (!started) {
        qWarning() << "Failed to start process:" << process->errorString();
    }

    delete process;
}