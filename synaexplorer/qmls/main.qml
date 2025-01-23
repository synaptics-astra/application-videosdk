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
        id: gettingStarted
        GettingStarted {}
    }

    Component {
        id: videoDemos1680
        VideoDemos1680 {}
    }

    Component {
        id: gfxDemos
        GfxDemos {}
    }

    Component {
        id: aiDemos
        AIDemos {}
    }

    Component {
        id: videoDemos1620
        VideoDemos1620 {}
    }

    Component {
        id: videoDemos1640
        VideoDemos1640 {}
    }

    Component {
        id: capabilityDemos
        CapabilityDemos {}
    }

    Component {
        id: settingsPanel
        Settings {}
    }

    Component {
        id: cameraApp
        CameraApp {}
    }

    function switchPage (index) {
        var machine = playbackControl.getMachine();
        switch (index) {
        case 0:
            sidemenu.sourceComponent = gettingStarted
            break
        case 1:
            if (machine === "sl1620") {
                sidemenu.sourceComponent = videoDemos1620
            } else if (machine === "sl1640") {
                sidemenu.sourceComponent = videoDemos1640
            } else if (machine === "sl1680") {
                sidemenu.sourceComponent = videoDemos1680
            }
            break
        case 2:
            sidemenu.sourceComponent = gfxDemos
            break
        case 3:
            sidemenu.sourceComponent = aiDemos
            break
        case 4:
            sidemenu.sourceComponent = capabilityDemos
            break
        case 5:
            sidemenu.sourceComponent = settingsPanel
            break
        case 6:
            if (machine === "sl1680") {
                sidemenu.sourceComponent = cameraApp
            }
            break
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
                Image {
                    source: "qrc:/res/images/vertical_line.png"
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            ListView {
                id: listItemView
                anchors.fill: parent
                highlightFollowsCurrentItem: true
                highlight: null
                spacing: Resource.width / 25
                property string machine: playbackControl.getMachine()
                function updateModel() {
                    var baseModel = ["Getting Started", "Video", "Graphics", "AI", "Capability Demo", "Settings"];
                    if (machine === "sl1680") {
                        baseModel.push("ISP Camera");
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
                    machine = playbackControl.getMachine();
                }

                onMachineChanged: {
                    listItemView.model = updateModel();
                }
            }
        }
    }

    Item {
        width: Resource.rightPanelWidth
        height: Resource.rightPanelHeight
        anchors.top: header.bottom
        anchors.left: sidePanel.right
        StackView {
            id: pageStackView
            anchors.fill: parent
            initialItem: Loader {
                id: sidemenu
                sourceComponent: gettingStarted
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

