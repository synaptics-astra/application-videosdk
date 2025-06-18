// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "playbackcontrol.h"

PlaybackControl::PlaybackControl()
{
    windowManager = new WindowManager(nullptr, this->machine);
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
    std::vector<std::string> arguments;
    arguments.push_back(path.toStdString());

    std::vector<std::string> environmentVariables;

#ifdef HAVE_WAYLAND
    environmentVariables.push_back("XDG_RUNTIME_DIR=/var/run/user/0");
    environmentVariables.push_back("WAYLAND_DISPLAY=wayland-1");
    environmentVariables.push_back("QT_QPA_PLATFORM=wayland");
#endif

#ifdef HAVE_X11
    environmentVariables.push_back("DISPLAY=:0");
    environmentVariables.push_back("QT_QPA_PLATFORM=xcb");
    environmentVariables.push_back("GST_GL_API=gles2");
#endif

    std::string scriptpath = path.toStdString();
    std::vector<char*> args;

    for (const auto& arg : arguments) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }

    if (path == "/usr/bin/syna-video-player") {
        args.push_back("--mach");
        args.push_back(const_cast<char*>(machine.c_str()));
        args.push_back("--mode");
        if (machine == "sl1620") {
            args.push_back("ffmpeg");
        } else {
            args.push_back("v4l2");
        }
    } else if (path == "/usr/bin/syna-ai-player"
            || path == "/usr/bin/syna-capability-demo") {
        args.push_back("--mach");
        args.push_back(const_cast<char*>(machine.c_str()));
    }
    args.push_back(nullptr);

    std::vector<char*> envs;
    for (const auto& envVar : environmentVariables) {
        envs.push_back(const_cast<char*>(envVar.c_str()));
    }
    envs.push_back(nullptr);

    char* const* argv = args.data();
    char* const* env = envs.data();

    pid_t pid;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return;
    }

    if (pid)  {
        return;
    }
    if (setsid() == -1)
        exit(EXIT_FAILURE);

    if (execve(scriptpath.c_str(), argv, env) < 0) {
        fprintf(stderr, "execl '%s' failed\n", strerror(errno));
        exit(1);
    }
}
