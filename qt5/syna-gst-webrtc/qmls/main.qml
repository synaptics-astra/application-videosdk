import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1
import Resource 1.0
import "./custom_views"
import "./panels"

ApplicationWindow {
    id: root
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: "FullScreen"
    title: qsTr("GStreamer WebRTC for Real-Time Communication")
    color: Resource.background

    Component.onCompleted: {
        if (!webrtcManager.hasInternetConnection()) {
            noInternetPopup.open()
        }
    }

    Popup {
        id: noInternetPopup
        anchors.centerIn: parent
        width: 400
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: "#ffffff"
            border.color: "#666666"
            border.width: 1
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 20
            anchors.fill: parent
            anchors.margins: 20

            Text {
                text: "No Internet Connection"
                font.pixelSize: 20
                font.bold: true
                color: "black"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Please check your internet connection and try again."
                font.pixelSize: 16
                color: "black"
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    noInternetPopup.close()
                    Qt.quit()
                }
                background: Rectangle {
                    radius: 8
                    color: "#003366"
                    border.color: "#002244"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    onVisibleChanged: {
        var wid = -1
        var targetScreen
        for (var i = 0; i < Qt.application.screens.length; i++) {
            if (wid < Qt.application.screens[i].width) {
                wid = Qt.application.screens[i].width
                targetScreen = Qt.application.screens[i]
            }
        }
        this.screen = targetScreen
        Resource.width = targetScreen ? targetScreen.width : root.width
        Resource.height = targetScreen ? targetScreen.height : root.height
    }

    header: Header {
        id: headerView
    }

    Item {
        id: spacer_0
        height: Resource.width/15
        width: 1
        anchors.top: headerView.bottom
    }


    Component {
        id: gstwebrtc
        GstWebRTC {}
    }

    Item {
        width: Resource.rightPanelWidth
        height: Resource.rightPanelHeight
        anchors.centerIn: parent
        anchors.topMargin: headerView.height + spacer_0.height
        StackView {
            id: pageStackView
            anchors.fill: parent
            initialItem: Loader {
                id: sidemenu
                sourceComponent: gstwebrtc
            }
        }
    }

    Button {
        id: exitButton
        width: root.width * 0.04
        height: root.height * 0.03
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: root.width * 0.01
        z: 2
        hoverEnabled: true
        scale: hovered ? 1.1 : 1.0
        Behavior on scale { NumberAnimation { duration: 200 } }

        background: Rectangle {
            color: parent.hovered ? "#0277bd" : "#0069d9"
            radius: 4
            Behavior on color { ColorAnimation { duration: 200 } }
        }

        contentItem: Text {
            text: "Exit"
            font.pixelSize: root.height * 0.013
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: {
            Qt.quit()
        }
    }
}