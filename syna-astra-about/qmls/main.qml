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
    title: qsTr("About")
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
        id: about
        About {}
    }

    Component {
        id: resources
        Resources {}
    }

    Component {
        id: oobeApps
        OobeApps {}
    }

    function switchPage (index) {
        switch (index) {
        case 0:
            sidemenu.sourceComponent = about
            break
        case 1:
            sidemenu.sourceComponent = oobeApps
            break
        case 2:
            sidemenu.sourceComponent = resources
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
                function updateModel() {
                    /* Adding new module */
                    var baseModel = ["About", "OOBE applications","Resources"];
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
        StackView {
            id: pageStackView
            anchors.fill: parent
            initialItem: Loader {
                id: sidemenu
                sourceComponent: about
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
