QT += quick gui-private widgets

CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 gstreamer-wayland-1.0 udev syna-player-framework"

SOURCES += main.cpp \
           playbackcontrol.cpp \
           filecontrol.cpp \

HEADERS += playbackcontrol.h \
           filecontrol.h \

RESOURCES += qml.qrc
