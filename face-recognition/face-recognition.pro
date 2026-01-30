QT += core gui
QT += gui-private widgets

CONFIG += c++17
CONFIG += debug
CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 syna-player-framework gstreamer-wayland-1.0 gstreamer-app-1.0 libudev opencv4"

SOURCES += \
    main.cpp \
    playerui.cpp \
    gstplayer.cpp \
    command.cpp \
    qtdevice.cpp \
    embedding.cpp

HEADERS += \
    playerui.h \
    gstplayer.h \
    controlpanel.h \
    preview.h \
    command.h \
    qtdevice.h \
    embedding.h \
    welcomepage.h \

RESOURCES += \
    app.qrc
