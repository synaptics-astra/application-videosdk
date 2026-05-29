QT += quick gui core
QT += gui-private widgets

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
    about.cpp \

HEADERS += \
    about.h \
    launcher.h \

RESOURCES += qml.qrc
