QT += core gui widgets

CONFIG += c++17
CONFIG += link_pkgconfig

PKGCONFIG += "opencv4 gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 libudev"

SOURCES += \
    main.cpp \
    processor.cpp \
    qtdevice.cpp

HEADERS += \
    processor.h \
    qtdevice.h

TARGET = opencv-gst-od
TEMPLATE = app

RESOURCES += app.qrc
