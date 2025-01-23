QT += widgets gui-private

CONFIG += link_pkgconfig
PKGCONFIG = "gstreamer-1.0 gstreamer-wayland-1.0 libudev gstreamer-plugins-base-1.0 gstreamer-rtsp-1.0 udev"

SOURCES += gstplayer.cpp \
           windowmanager.cpp \
           settingsmanager.cpp \

HEADERS += gstplayer.h \
           windowmanager.h \
           settingsmanager.h \

TEMPLATE = lib
CONFIG += shared
