import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"

Rectangle {
    id: root
    width: 200
    height: 180
    color: Resource.background

    Item {
        width: parent.width
        height: 40
    }

    Rectangle {
        id: wifiBar
        width: 200
        height: 60
        color: "#007DC3"
        radius: 12
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 80
        anchors.margins: 5

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 0

            Text {
                text: qsTr("Wi-Fi")
                font.pixelSize: 22
                color: "#FFFFFF"
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                width: 80
                height: 36
                radius: 18
                color: wifiToggle.checked ? "#4CAF50" : "#B0BEC5"
                border.color: "#FFFFFF"
                border.width: 2
                Layout.alignment: Qt.AlignVCenter

                Rectangle {
                    id: toggleCircle
                    width: 32
                    height: 32
                    radius: 16
                    color: "white"
                    anchors.verticalCenter: parent.verticalCenter
                    x: wifiToggle.checked ? parent.width - width - 2 : 2

                    Behavior on x {
                        NumberAnimation {
                            duration: 200
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        wifiToggle.checked = !wifiToggle.checked
                        wifiapp.togglePythonScript(wifiToggle.checked)
                    }
                }
            }
        }
    }

    Rectangle {
        id: availableNetworks
        width: parent.width * 0.5
        height: 60
        color: "#007DC3"
        radius: 12
        opacity: wifiToggle.checked ? 1 : 0
        visible: true
        anchors.top: wifiBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 0

            Text {
                text: qsTr("Available Networks")
                font.pixelSize: 20
                color: "#FFFFFF"
                Layout.alignment: Qt.AlignVCenter
            }

            BusyIndicator {
                id: loadingIndicator
                visible: false
                running: visible
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                loadingIndicator.visible = true
                wifiapp.showAvailableNetworks()
                loadingTimer.start()
            }
        }

        Timer {
            id: loadingTimer
            interval: 4300
            onTriggered: loadingIndicator.visible = false
        }
    }

    Rectangle {
        id: manageNetworks
        width: parent.width * 0.5
        height: 60
        color: "#007DC3"
        radius: 12
        opacity: wifiToggle.checked ? 1 : 0
        visible: true
        anchors.top: availableNetworks.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 0

            Text {
                text: qsTr("Manage Known Networks")
                font.pixelSize: 20
                color: "#FFFFFF"
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: wifiapp.manageKnownNetworks()
        }
    }

    signal toggleButtonClicked(bool state)

    property alias wifiState: wifiToggle.checked

    Switch {
        id: wifiToggle
        checked: wifiapp.wifiState
        visible: false
        onCheckedChanged: wifiapp.togglePythonScript(checked)
    }
}
