import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1

import Resource 1.0

import "./custom_views"
import "./panels"

ApplicationWindow {
    id: root
    width: 1920
    height: 1080
    visible: true
    visibility: "FullScreen"
    title: qsTr("Syna Explorer")
    color: Resource.background

    onVisibleChanged: {
        var maxWidth = -1
        var targetScreen
        for (var i = 0; i < Qt.application.screens.length; i++) {
            if (maxWidth < Qt.application.screens[i].width) {
                maxWidth = Qt.application.screens[i].width
                targetScreen = Qt.application.screens[i]
            }
        }
        root.screen = targetScreen
        Resource.width = targetScreen.width
        Resource.height = targetScreen.height
    }

    header: Header {
        id: headerView
    }

    Item {
        id: spacer0
        height: Resource.width / 15
        width: 1
        anchors.top: headerView.bottom
    }

    Component {
        id: home
        Home {}
    }
    /* Adding new module */

    Component {
        id: sampleModule
        SampleModule {}
    }

    Component {
        id: panelSelection
        PanelSelection {}
    }

    Component {
        id: wifi
        Wifi {}
    }

    Component {
        id: bluetooth
        Bluetooth {}
    }

    Component {
        id: usbformatpriorities
        UsbFormatPriorities {}
    }

    Component {
        id: rtspConfigComponent
        RtspConfig {}
    }

    function switchPage (index) {
        if (machine === "sl2619") {
            switch (index) {
            case 0:
                sidemenu.sourceComponent = home
                break
            case 1:
                sidemenu.sourceComponent = usbformatpriorities
                break
            case 2:
                sidemenu.sourceComponent = rtspConfigComponent
                break
            }
        }
        else {
            switch (index) {
            case 0:
                sidemenu.sourceComponent = home
                break

            case 1:
                sidemenu.sourceComponent = panelSelection
                break
            /* Add new module before sample module*/

            case 1:
                sidemenu.sourceComponent = sampleModule
                break
            /* Adding new module */
            case 2:
                sidemenu.sourceComponent = wifi
                break
            case 3:
                sidemenu.sourceComponent = bluetooth
                break
            case 4:
                sidemenu.sourceComponent = usbformatpriorities
                break
            case 5:
                sidemenu.sourceComponent = rtspConfigComponent
                break
            }
        }
    }

    RowLayout {
        id: sidePanel
        anchors.top: spacer0.bottom
        height: parent.height - headerView.height - spacer0.height

        Pane {
            padding: 0
            Layout.preferredWidth: Resource.leftPanelWidth
            Layout.fillHeight: true
            background: Rectangle {
                anchors.fill: parent
                color: Resource.background
            }

            ListView {
                ScrollBar.vertical: ScrollBar {
                    id: scrollBar
                    width: 10
                    policy: ScrollBar.AlwaysOn
                    background: Rectangle {
                        color: "#d6e9ff" // background color
                        border.color: "#cccccc" // border color
                        border.width: 1
                        radius: 5
                    }
                    contentItem: Rectangle {
                        color: "#042757" // handle color
                        border.color: "#ffffff" // handle border color
                        border.width: 1
                        radius: 5
                    }
                }
                id: listItemView
                anchors.fill: parent
                highlightFollowsCurrentItem: true
                highlight: null
                spacing: Resource.width / 25
                function updateModel() {
                    if (machine === "sl2619") {
                        var baseModel = ["Home", "USB Camera", "RTSP Camera"];
                    } else {
                        var baseModel = ["Home", "Display", "Wi-Fi", "Bluetooth", "USB Camera", "RTSP Camera"]
                        /* Adding new module */
                        if (isSampleModuleEnabled)
                            baseModel.push("Sample Module");
                    }

                    return baseModel;
                }

                model: updateModel()

                delegate: ListItem {
                    spacing: Resource.width / 36
                    title: modelData
                    width: listItemView.width
                    onClicked: {
                        ListView.view.currentIndex = index
                        root.switchPage(index)
                    }
                }

                Component.onCompleted: {
                    updateModel();
                }
            }
        }
    }

    Item {
        width: Resource.rightPanelWidth
        height: Resource.rightPanelHeight
        anchors.top: header.bottom
        anchors.left: sidePanel.right
        anchors.leftMargin: 10
        StackView {
            id: pageStackView
            anchors.fill: parent
            initialItem: Loader {
                id: sidemenu
                sourceComponent: home
            }
        }
    }

    Button {
        id: exitButton
        anchors.bottom: parent.bottom
        opacity: 0.87
        text: "Exit"
        width: listItemView.width
        font.pixelSize: Resource.h4
        font.family: Resource.vollkornRegular.name
        bottomPadding: Resource.width / 45
        onClicked: {
            Qt.quit()
        }
        background: Rectangle {
            color: "transparent"
            anchors.fill: parent
            radius: 8
        }
    }
}

