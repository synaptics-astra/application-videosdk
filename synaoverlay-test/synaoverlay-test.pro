QT += core gui widgets

CONFIG += c++17
CONFIG += debug
CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 syna-player-framework opencv4"
PKGCONFIG += "egl glesv2"

wayland {
    message("Configuring for Wayland support")
    PKGCONFIG += gstreamer-wayland-1.0
    DEFINES += HAVE_WAYLAND
}

SOURCES += \
    main.cpp \
    gstplayer.cpp \
    overlaytestpanel.cpp

HEADERS += \
    gstplayer.h \
    overlaytestpanel.h

RESOURCES += app.qrc
