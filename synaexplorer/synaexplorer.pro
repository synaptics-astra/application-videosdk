QT += quick gui core
QT += gui-private widgets

CONFIG += link_pkgconfig

PKGCONFIG = "gstreamer-1.0 gstreamer-wayland-1.0 udev syna-player-framework "

SOURCES += \
    main.cpp \
    playbackcontrol.cpp \
    wifiapp.cpp \

HEADERS += \
    playbackcontrol.h \
    wifiapp.h \


RESOURCES += qml.qrc

