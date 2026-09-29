QT += quick gui-private widgets


CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0 udev syna-player-framework

# Platform-specific configuration
wayland {
    message("Configuring for Wayland support")
    PKGCONFIG += gstreamer-wayland-1.0
    DEFINES += HAVE_WAYLAND
}

x11 {
    message("Configuring for X11 support")
    PKGCONFIG += x11
    DEFINES += HAVE_X11
}

SOURCES += main.cpp \
           playbackcontrol.cpp \
           devfreqmanager.cpp \

HEADERS += playbackcontrol.h \
           devfreqmanager.h \


RESOURCES += app.qrc
