// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "playbackcontrol.h"

PlaybackControl::PlaybackControl(string machine) : machine(machine)
{
    windowManager = new WindowManager(nullptr, this->machine, "Synaptics AI Player");
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
    environmentVariables.push_back("XDG_RUNTIME_DIR=/var/run/user/0");
    environmentVariables.push_back("WAYLAND_DISPLAY=wayland-1");
    environmentVariables.push_back("QT_QPA_PLATFORM=wayland");

    std::string scriptpath = path.toStdString();
    std::vector<char*> args;
    for (const auto& arg : arguments) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);

    std::vector<char*> envs;
    for (const auto& envVar : environmentVariables) {
        envs.push_back(const_cast<char*>(envVar.c_str()));
    }
    envs.push_back(nullptr);

    char* const* argv = args.data();
    char* const* env = envs.data();

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return;
    }

    if (pid) {
        return;
    }

    if (setsid() == -1) {
        exit(EXIT_FAILURE);
    }

    if (execve(scriptpath.c_str(), argv, env) < 0) {
        fprintf(stderr, "execve '%s' failed: %s\n", scriptpath.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }
}
