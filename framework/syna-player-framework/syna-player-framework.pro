QT += widgets gui-private

CONFIG += link_pkgconfig

PKGCONFIG = "gstreamer-1.0 libudev gstreamer-plugins-base-1.0 gstreamer-rtsp-1.0 udev syna-settings-framework gstreamer-video-1.0 "
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
           statswidget.cpp \

HEADERS += syna-player-common.h \
           gstplayer.h \
           windowmanager.h \
           settingsmanager.h \
           filedialoghelper.h \
           videoprober.h \
           processhelper.h \
           statswidget.h \

RESOURCES += synaplayerframework.qrc

TEMPLATE = lib
CONFIG += shared
