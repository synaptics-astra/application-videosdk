import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1

import Resource 1.0

import "./custom_views"
import "./panels"
import PageConfig 1.0

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
        id: aiDemos2619
        AIDemos2619 {}
    }

    Component {
        id: aiDemos2615
        AIDemos2615 {}
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
        id: videoDemos2619
        VideoDemos2619 {}
    }

    Component {
        id: videoDemos2615
        VideoDemos2615 {}
    }

    Component {
        id: capabilityDemos
        CapabilityDemos {
            visible: !isX11
        }
    }

    Component {
        id: cameraApp
        CameraApp {}
    }

    Component {
        id: realTimeStreaming
        RealTimeStreaming {}
    }

    function switchPage(index) {
        var machine = playbackControl.getMachine()
        var componentName = PageConfig.getComponent(machine, index, isX11)
        if (componentName) {
            sidemenu.sourceComponent = eval(componentName)
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
                    width: Resource.width / 160
                    policy: ScrollBar.AlwaysOn
            // Force overlay positioning
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

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
                        radius: scrollBar.width / 2
                    }
                }

                id: listItemView
                anchors.fill: parent
                highlightFollowsCurrentItem: true
                highlight: null
                spacing: Resource.width / 25
                property string machine: playbackControl.getMachine()
                function updateModel() {
                    return PageConfig.getMenuModel(machine, isX11)
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
                    machine = playbackControl.getMachine()
                }

                onMachineChanged: {
                    listItemView.model = updateModel()
                }
            }
        }
    }

    Item {
        width: Resource.rightPanelWidth
        height: Resource.rightPanelHeight
        anchors.top: headerView.bottom
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
