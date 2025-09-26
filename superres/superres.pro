QT += core gui
QT += gui-private widgets

CONFIG += c++17
CONFIG += link_pkgconfig
PKGCONFIG += "gstreamer-1.0 gstreamer-wayland-1.0 libudev syna-player-framework wayland-client"

SOURCES += \
    main.cpp \
    playerui.cpp \
    gstplayer.cpp \
    command.cpp \
    qtdevice.cpp

HEADERS += \
    playerui.h \
    gstplayer.h \
    controlpanel.h \
    settingsdialog.h \
    command.h \
    qtdevice.h \

RESOURCES += \
    app.qrc