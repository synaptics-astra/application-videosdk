import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../elements"

Rectangle {
    id: root
    width: 200
    height: 180
    color: Resource.background

    PageTitle {
        text: qsTr("Bluetooth")
        Layout.alignment: Qt.AlignHCenter
    }

    Item {
        width: parent.width
        height: 40
    }

    RowBar {
        id: bluetoothBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 80

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 0

            Text {
                text: qsTr("Bluetooth")
                font.pixelSize: 22
                color: Resource.whiteColor
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            ToggleButton {
                id: bluetoothToggleButton
                checked: bluetoothapp.bluetoothState

                onToggleStateChanged: {
                    bluetoothapp.togglePythonScript(checked)
                }
            }
        }
    }

    RowBar {
        id: availableDevices
        opacity: bluetoothToggle.checked ? 1 : 0
        visible: true
        anchors.top: bluetoothBar.bottom
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
                text: qsTr("Available Devices")
                font.pixelSize: 20
                color: Resource.whiteColor
                Layout.alignment: Qt.AlignVCenter
            }

            LoadingIndicator {
                id: loadingIndicator
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 10
                size: 48
                color: Resource.whiteColor
            }

        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                loadingIndicator.visible = true
                bluetoothapp.showAvailableDevices()
                loadingTimer.start()
            }
        }

        Timer {
            id: loadingTimer
            interval: 4300
            onTriggered: loadingIndicator.visible = false
        }
    }

    RowBar {
        id: manageDevices
        opacity: bluetoothToggle.checked ? 1 : 0
        visible: true
        anchors.top: availableDevices.bottom
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
                text: qsTr("Manage Known Devices")
                font.pixelSize: 20
                color: Resource.whiteColor
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: bluetoothapp.manageKnownDevices()
        }
    }

    signal toggleButtonClicked(bool state)

    property alias bluetoothState: bluetoothToggle.checked

    Switch {
        id: bluetoothToggle
        checked: bluetoothapp.bluetoothState
        visible: false
        onCheckedChanged: bluetoothapp.togglePythonScript(checked)
    }
}
