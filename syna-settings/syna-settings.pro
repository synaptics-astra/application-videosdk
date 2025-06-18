QT += quick gui core
QT += gui-private widgets

CONFIG += link_pkgconfig

PKGCONFIG = "syna-settings-framework "

SOURCES += \
    main.cpp \
    DisplaySettings.cpp \
    wifiapp.cpp \
    bluetoothapp.cpp

HEADERS += \
    settingscontrol.h \
    settingsdata.h \
    DisplaySettings.h \
    wifiapp.h \
    bluetoothapp.h

RESOURCES += qml.qrc
