QT += quick gui core
QT += gui-private widgets

CONFIG += link_pkgconfig

PKGCONFIG = "syna-settings-framework "

SOURCES += \
    main.cpp \
    DisplaySettings.cpp

HEADERS += \
    settingscontrol.h \
    settingsdata.h \
    DisplaySettings.h

RESOURCES += qml.qrc
