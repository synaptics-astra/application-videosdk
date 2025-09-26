QT += quick gui core
QT += gui-private widgets

CONFIG += link_pkgconfig

PKGCONFIG = "syna-settings-framework gstreamer-1.0 gstreamer-rtsp-1.0 glib-2.0"
# Platform-specific configuration
wayland {
    message("Configuring for Wayland support")
    DEFINES += HAVE_WAYLAND
}

x11 {
    message("Configuring for X11 support")
    DEFINES += HAVE_X11
}

SOURCES += \
    main.cpp \
    DisplaySettings.cpp \
    wifiapp.cpp \
    bluetoothapp.cpp \
    rtspconfig.cpp   \

HEADERS += \
    settingscontrol.h \
    settingsdata.h \
    DisplaySettings.h \
    wifiapp.h \
    bluetoothapp.h \
    rtspconfig.h  \

RESOURCES += qml.qrc
