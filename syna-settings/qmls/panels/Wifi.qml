import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"
import "../elements"

Rectangle {
    id: root
    width: 200
    height: 180
    color: Resource.background

    PageTitle {
        text: qsTr("Wi-Fi")
        Layout.alignment: Qt.AlignHCenter
    }

    Item {
        width: parent.width
        height: 40
    }

    RowBar {
        id: wifiBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 80

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 0

            Text {
                text: qsTr("Wi-Fi")
                font.pixelSize: 22
                color: Resource.whiteColor
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            ToggleButton {
                id: wifiToggleButton
                checked: wifiapp.wifiState
                property bool initialized: false

                Component.onCompleted: initialized = true

                onToggleStateChanged: {
                    if (initialized) {
                        wifiapp.togglePythonScript(checked)
                    }
                }
            }
        }
    }

    RowBar {
        id: availableNetworks
        opacity: wifiToggleButton.checked ? 1 : 0
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
                color: Resource.whiteColor
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

    RowBar {
        id: manageNetworks
        opacity: wifiToggleButton.checked ? 1 : 0
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
                color: Resource.whiteColor
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: wifiapp.manageKnownNetworks()
        }
    }

    signal toggleButtonClicked(bool state)

    property alias wifiState: wifiToggleButton.checked

}
