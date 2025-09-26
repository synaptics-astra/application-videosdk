QT += quick gui-private widgets network

CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 udev"

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
           webrtc_manager.cpp \

HEADERS += webrtc_manager.h \

RESOURCES += qml.qrc
