QT += core gui network
QT += gui-private widgets

CONFIG += c++17
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0 gstreamer-video-1.0 libudev syna-player-framework
LIBS += -lgstapp-1.0

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

SOURCES += \
    main.cpp \
    playerui.cpp \
    gstplayer.cpp \
    command.cpp \

HEADERS += \
    playerui.h \
    gstplayer.h \
    controlpanel.h \
    settingsdialog.h \
    command.h \
    filepopup.h \
    validationpopup.h \
    samplesrcpopup.h \

RESOURCES += \
    app.qrc