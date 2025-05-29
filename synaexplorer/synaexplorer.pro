QT += quick gui core
QT += gui-private widgets

CONFIG += link_pkgconfig

PKGCONFIG = "gstreamer-1.0 udev syna-player-framework "

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

SOURCES += \
    main.cpp \
    playbackcontrol.cpp \

HEADERS += \
    playbackcontrol.h \


RESOURCES += qml.qrc

