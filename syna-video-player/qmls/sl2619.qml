import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.3

ApplicationWindow {
    id: player
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: "FullScreen"
    title: qsTr("Syna Video Player - SL2619")
    color: "#ffffff"

    property var cmdsModel: null
    property var rtspUrlsModel: null
    property string url1: ""
    property int settingsIdRtspUrls: 2 // Corrected to 1 based on SettingsManager.cpp enum

    Component.onCompleted: {
        var cmdComponent
        var mode = playbackControl.getMode()
        if (mode === "ffmpeg") {
            cmdComponent = Qt.createComponent("file:///home/root/demos/qmls/ffmpegcommands.qml")
        }
        cmdsModel = cmdComponent.createObject()

        try {
            var rtspData = settingsManager.getData(settingsIdRtspUrls)
            rtspUrlsModel = rtspData && rtspData["rtspUrls"] ? rtspData["rtspUrls"] : [""]
        } catch (error) {
            console.error("Error in settingsManager.getData:", error.message)
            rtspUrlsModel = [""]
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
        player.screen = targetScreen
    }

    Popup {
        id: dialog
        modal: true
        focus: true
        anchors.centerIn: player.contentItem

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
                        text: "RTSP URL 1"
                        Layout.fillWidth: true
                    }

                    TextField {
                        text: rtspUrlsModel && rtspUrlsModel.length > 0 ? rtspUrlsModel[0] : ""
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
            var urls = []
            var row = rowLayout
            if (row) {
                for (var j = 0; j < row.children.length; j++) {
                    var child = row.children[j]
                    if (child.hasOwnProperty("placeholderText")) {
                        urls.push(child.text)
                    }
                }
            } else {
                console.error("Row not found")
            }
            var rtspData = { "rtspUrls": urls }
            try {
                var success = settingsManager.setData(settingsIdRtspUrls, rtspData)
                if (!success) {
                    console.error("settingsManager.setData failed")
                }
            } catch (error) {
                console.error("Error in settingsManager.setData:", error.message)
            }
        }
    }

    ColumnLayout {
        id: mainLayout
        spacing: 10
        width: player.width

        Rectangle {
            color: "black"
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            height: player.height / 12

            Image {
                source: "qrc:/images/astra_white.png"
                sourceSize: Qt.size(100, 100)
            }

            Label {
                id: label
                text: "Astra Video Playback Demo"
                font.pixelSize: player.height / 25
                font.bold: true
                color: "white"
                anchors.centerIn: parent
            }
        }

        RowLayout {
            id: secondLayer
            spacing: 10
            Layout.leftMargin: player.height / 25
            Layout.rightMargin: player.height / 25
            Layout.topMargin: player.height / 35
            Layout.bottomMargin: 0
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft

            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true
                Repeater {
                    model: 1
                    delegate: Label {
                        text: "VIDEO " + (index + 1)
                        font.pixelSize: 30
                        Layout.fillWidth: true
                        Layout.preferredHeight: player.height / 15
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        background: Rectangle {
                            opacity: enabled ? 1 : 0.3
                            color: "#6495ed"
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true

                Repeater {
                    model: 1
                    delegate: ComboBox {
                        id: comboBox
                        textRole: "text"
                        valueRole: "value"
                        model: player.cmdsModel
                        font.pixelSize: 30
                        width: parent.width * 0.8
                        Layout.fillWidth: true
                        Layout.preferredHeight: player.height / 15
                        delegate: ItemDelegate {
                            font.pixelSize: 30
                            width: comboBox.width
                            contentItem: Text {
                                text: model.text
                                font: comboBox.font
                            }
                        }

                        onCurrentIndexChanged: {
                            player["url" + (index + 1)] = valueAt(currentIndex)
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.topMargin: player.height / 25
            Layout.fillWidth: true
            Button {
                id: update
                text: "Update RTSP Urls"
                font.pixelSize: player.height / 35
                highlighted: true
                Layout.leftMargin: player.height / 25
                background: Rectangle {
                    color: update.down ? "#000000" : "#a9a9a9"
                    radius: 20
                }

                onClicked: {
                    dialog.open()
                }
            }
        }

        RowLayout {
            Layout.topMargin: player.height / 25
            Layout.preferredWidth: parent.width / 4
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            RadioButton {
                id: singleView
                text: "SINGLE VIEW"
                font.pixelSize: player.height / 35
                Layout.alignment: Qt.AlignCenter
                checked: true
            }
        }

        ColumnLayout {
            Layout.topMargin: player.height / 25
            Layout.leftMargin: player.height / 25
            Layout.rightMargin: player.height / 25
            Layout.preferredWidth: parent.width / 2
            Layout.alignment: Qt.AlignHCenter

            Button {
                id: play
                text: "PLAY"
                font.pixelSize: player.height / 35
                highlighted: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                Layout.preferredHeight: player.height / 18
                background: Rectangle {
                    opacity: enabled ? 1 : 0.3
                    color: play.down ? "#000000" : "#a9a9a9"
                }
                onClicked: {
                    if (singleView.checked) {
                        playbackControl.doPlayback(1, player.url1, "", "", "")
                    }
                }
            }

            Button {
                id: exit
                text: "Exit"
                font.pixelSize: player.height / 35
                highlighted: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                Layout.preferredHeight: player.height / 18
                background: Rectangle {
                    opacity: enabled ? 1 : 0.3
                    color: exit.down ? "#000000" : "#a9a9a9"
                }
                onClicked: Qt.quit()
            }
        }
    }
}

