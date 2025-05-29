QT += widgets gui-private

CONFIG += link_pkgconfig

PKGCONFIG = "gstreamer-1.0 libudev gstreamer-plugins-base-1.0 gstreamer-rtsp-1.0 udev"
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


SOURCES += gstplayer.cpp \
           windowmanager.cpp \
           settingsmanager.cpp \
           filedialoghelper.cpp \
           videoprober.cpp \
           processhelper.cpp \

HEADERS += gstplayer.h \
           windowmanager.h \
           settingsmanager.h \
           filedialoghelper.h \
           videoprober.h \
           processhelper.h \

TEMPLATE = lib
CONFIG += shared
