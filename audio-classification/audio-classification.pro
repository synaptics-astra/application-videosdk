QT += quick widgets

CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 gstreamer-app-1.0 glib-2.0 gobject-2.0"

SOURCES += \
           appsink_qml.cpp \
           main.cpp

RESOURCES += qml.qrc

HEADERS += \
           appsink_qml.h
