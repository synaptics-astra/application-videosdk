QT += widgets gui-private

CONFIG += link_pkgconfig
PKGCONFIG = "gstreamer-1.0 gstreamer-wayland-1.0 libudev"

SOURCES += gstplayer.cpp \
           windowmanager.cpp \

HEADERS += gstplayer.h \
           windowmanager.h \

TEMPLATE = lib
CONFIG += shared
