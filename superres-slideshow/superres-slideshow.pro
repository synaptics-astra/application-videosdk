QT += quick gui core
QT += gui-private widgets
CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 gstreamer-app-1.0 glib-2.0 gobject-2.0 syna-player-framework"

SOURCES += \
        frameprovider.cpp \
        main.cpp

RESOURCES += qml.qrc

HEADERS += \
        frameprovider.h \
        filemanager.h
