import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import Resource 1.0
import "../custom_views"

Pane {
    id: gridItem
    property int type: 0
    property string image: "qrc:/res/images/astra_icon.png"
    property string title: qsTr("Syna AI Player")
    property string command1: "videotestsrc ! waylandsink"
    property string command2: "videotestsrc ! waylandsink"
    property string command3: "videotestsrc ! waylandsink"
    property string command4: "videotestsrc ! waylandsink"

    width: Resource.rightPanelWidth/2
    height: Resource.rightPanelWidth/2

    property var hostIP: null
    Component.onCompleted: {
        hostIP = settingsManager.getHostIP()
        hostIP !== null ? hostIP : ""
    }

    Popup {
        id: dialog
        modal: true
        focus: true
        anchors.centerIn: gridItem.contentItem

        contentItem: Item {
            ColumnLayout {
                spacing: 10
                Layout.alignment: Qt.AlignCenter

                Rectangle {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 20
                    color: "#f0f0f0"
                    border.color: "#ccc"
                    Text {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignCenter
                        text: "Edit URLs"
                        font.bold: true
                    }
                }

                RowLayout {
                    id: rowLayout
                    spacing: 10
                    Layout.fillWidth: true

                    Text {
                        text: "HOST IP "
                        Layout.fillWidth: true
                    }

                    TextField {
                        text: hostIP
                        placeholderText: "Enter URL"
                        background: Rectangle {
                            color: "white"
                            border.color: "#3498db"
                            border.width: 1
                            radius: 4
                        }
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    spacing: 10
                    width: parent.width
                    Button {
                        text: "OK"
                        highlighted: true
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: update.down ? "#000000" : "#a9a9a9"
                            radius: 20
                        }
                        onClicked: dialog.close()
                    }
                }
            }
        }

        onClosed: {
            var url
            for (var j = 0; j < rowLayout.children.length; j++) {
                var child = rowLayout.children[j];
                if (child.hasOwnProperty("placeholderText")) {
                    url = child.text
                }
            }
            settingsManager.setHostIP(url)
            settingsManager.saveConfig("/home/root/demos/configs/synaexplorer_config.txt")
            gridItem.command1 = gridItem.command1.replace("HOST_IP", url)
            playbackControl.doPlayback(1, gridItem.command1, "", "", "")
        }
    }

    background: Rectangle {
        anchors.fill: parent
        color : "transparent"
        radius: 15
    }

    ColumnLayout {
        Image {
            source: gridItem.image
            sourceSize: Qt.size(Resource.rightPanelWidth/2, Resource.rightPanelWidth/2)
            MouseArea {
                width: parent.width
                height: parent.height
                focus: true
                anchors.fill: parent
                onClicked: {
                    switch (gridItem.type) {
                    case 1:
                        playbackControl.doPlayback(gridItem.type, gridItem.command1, "", "", "")
                        break;
                    case 2:
                        playbackControl.doPlayback(gridItem.type, gridItem.command1, gridItem.command2, gridItem.command3, gridItem.command4)
                        break;
                    case 3:
                        dialog.open()
                        break;
                    default:
                        playbackControl.doPlayback(1, gridItem.command1, gridItem.command2, gridItem.command3, gridItem.command4)
                        break;
                    }
                }
            }
        }

        TextView {
            text: title
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
