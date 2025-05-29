import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.1

import Resource 1.0
import "../custom_views"

Pane {
    id: gridItem
    property int type: 0
    property string mode: "default"
    property string image: "qrc:/res/images/astra_icon.png"
    property string title: qsTr("Syna AI Player")
    property string modelPipeline: ""
    property string command1: "videotestsrc ! placeholdersink"
    property string command2: "videotestsrc ! placeholdersink"
    property string command3: "videotestsrc ! placeholdersink"
    property string command4: "videotestsrc ! placeholdersink"

    width: Resource.rightPanelWidth/2
    height: Resource.rightPanelWidth/2

    property var hostIP: null
    property real rootWidth: Window.window ? Window.window.width : Resource.width
    property real rootHeight: Window.window ? Window.window.height : Resource.height

    Component.onCompleted: {
        hostIP = settingsManager.getHostIP()
        hostIP !== null ? hostIP : ""
    }

    Popup {
        id: resolutionPopup
        width: 300
        height: 100
        modal: false
        focus: true
        x: iconImage.x + iconImage.width + 10
        y: iconImage.y + (iconImage.height - height) / 2
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "#f8d7da"
            border.color: "#f5c2c7"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Resolution above 1080p is not supported."
                color: "#721c24"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Timer {
            id: autoCloseTimer
            interval: 3000
            running: false
            repeat: false
            onTriggered: resolutionPopup.close()
        }

        onOpened: autoCloseTimer.start()
    }

    Popup {
        id: resolutionSwitchPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.max(0.20 * gridItem.rootWidth, 250)
        height: Math.max(0.15 * gridItem.rootHeight, 120)

        background: Rectangle {
            color: "#f0f0f0"
            radius: gridItem.rootWidth / 240
            border.color: "#ccc"
        }

        contentItem: ColumnLayout {
            spacing: gridItem.rootWidth / 120
            anchors.fill: parent
            anchors.margins: gridItem.rootWidth / 120

            Text {
                text: "To run this application, we need to switch to 4K UI. Press OK to continue."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: gridItem.rootWidth / 90
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: gridItem.rootWidth / 120

                Button {
                    text: "Ok"
                    Layout.fillWidth: true
                    contentItem: Text {
                        text: parent.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: gridItem.rootWidth / 110
                    }
                    onClicked: {
                        var command = "mv /etc/modprobe.d/syna_drm.conf /home/syna_drm.conf && sync && reboot";
                        processHelper.executeCommand(command);
                        resolutionSwitchPopup.close()
                    }
                }

                Button {
                    text: "Cancel"
                    Layout.fillWidth: true
                    contentItem: Text {
                        text: parent.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: gridItem.rootWidth / 110
                    }
                    onClicked: {
                        resolutionSwitchPopup.close()
                    }
                }
            }
        }
    }

    Popup {
        id: resolutionSwitchBackPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.max(0.20 * gridItem.rootWidth, 250)
        height: Math.max(0.15 * gridItem.rootHeight, 120)

        background: Rectangle {
            color: "#f0f0f0"
            radius: gridItem.rootWidth / 240
            border.color: "#ccc"
        }

        contentItem: ColumnLayout {
            spacing: gridItem.rootWidth / 120
            anchors.fill: parent
            anchors.margins: gridItem.rootWidth / 120

            Text {
                text: "To run this application, we need to switch to 2k UI. Press OK to continue."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: gridItem.rootWidth / 90
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: gridItem.rootWidth / 120

                Button {
                    text: "Ok"
                    Layout.fillWidth: true
                    contentItem: Text {
                        text: parent.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: gridItem.rootWidth / 110
                    }
                    onClicked: {
                        var command = "mv /home/syna_drm.conf /etc/modprobe.d/syna_drm.conf && sync && reboot";
                        processHelper.executeCommand(command);
                        resolutionSwitchBackPopup.close()
                    }
                }

                Button {
                    text: "Cancel"
                    Layout.fillWidth: true
                    contentItem: Text {
                        text: parent.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: gridItem.rootWidth / 110
                    }
                    onClicked: {
                        resolutionSwitchBackPopup.close()
                    }
                }
            }
        }
    }

    Popup {
        id: frameratePopup
        width: 300
        height: 100
        modal: false
        focus: true
        x: iconImage.x + iconImage.width + 10
        y: iconImage.y + (iconImage.height - height) / 2
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "#f8d7da"
            border.color: "#f5c2c7"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Framerate greater than 25fps is not supported."
                color: "#721c24"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Timer {
            id: framerateAutoCloseTimer
            interval: 3000
            running: false
            repeat: false
            onTriggered: frameratePopup.close()
        }

        onOpened: framerateAutoCloseTimer.start()
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

    FileDialog {
        id: fileDialog
        title: "Select a Video File"
        nameFilters: ["Video files (*.mp4 *.h265 *.h264)"]
        modality: Qt.WindowModal

        onAccepted: {
            var selectedFile = fileDialog.fileUrl.toString().replace("file://", "")
            var videoInfo = videoProber.probeFile(selectedFile)

            // Resolution check
            if (videoInfo.width > 1920 || videoInfo.height > 1080) {
                resolutionPopup.open()
                return
            }

            // Framerate check
            var framerateValue = 0
            if (videoInfo.framerate && videoInfo.framerate.includes("/")) {
                framerateValue = parseInt(videoInfo.framerate.split("/")[0])
            } else {
                console.log("Framerate invalid or missing, defaulting to 0")
            }
            if (framerateValue > 25) {
                frameratePopup.open()
                return
            }

            var decoder = ""
            var demuxer = ""

            // Demuxer based on container
            if (videoInfo.container.toLowerCase() === "mp4") {
                demuxer = "qtdemux name=demux demux.video_0 ! queue"
            } else if (!videoInfo.container) {
                demuxer = "queue";
            } else {
                console.log("Unsupported container: " + videoInfo.container)
                return
            }

            // Decoder based on codec
            if (videoInfo.codec === "h264") {
                decoder = "h264parse ! v4l2h264dec"
            } else if (videoInfo.codec === "h265") {
                decoder = "h265parse ! v4l2h265dec"
            } else {
                console.log("Unsupported codec: " + videoInfo.codec)
                return
            }

            // Final command
            gridItem.command1 = "filesrc location=" + selectedFile + " ! " +
                                demuxer + " ! " + decoder + " ! " + gridItem.modelPipeline

            playbackControl.doPlayback(1, gridItem.command1, "", "", "")
        }

    }

    background: Rectangle {
        anchors.fill: parent
        color: "transparent"
        radius: 15
    }

    ColumnLayout {
        Image {
            id: iconImage
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
                            if (gridItem.mode === "file") {
                                if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                    resolutionSwitchBackPopup.open()
                                } else {
                                    fileDialog.open()
                                }
                            } else if (gridItem.mode === "camera") {
                                if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                    resolutionSwitchBackPopup.open()
                                } else {
                                    playbackControl.doPlayback(gridItem.type, gridItem.command1, "", "", "")
                                }
                            }
                            break;
                        case 2:
                            if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                resolutionSwitchBackPopup.open()
                            } else {
                                playbackControl.doPlayback(gridItem.type, gridItem.command1, gridItem.command2, gridItem.command3, gridItem.command4)
                            }
                            break;
                        case 3:
                            dialog.open()
                            break;
                        case 4:
                            if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                playbackControl.launch("/usr/bin/superres-slideshow")
                            } else {
                                resolutionSwitchPopup.open()
                            }
                            break;
                        case 5:
                            if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                playbackControl.launch("/usr/bin/superres")
                            } else {
                                resolutionSwitchPopup.open()
                            }
                            break;
                        case 6:
                        if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                    resolutionSwitchBackPopup.open()
                                } else {
                                playbackControl.launch("/usr/bin/face-recognition")
                                }
                            break;
                        default:
                            if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                                playbackControl.doPlayback(1, gridItem.command1, gridItem.command2, gridItem.command3, gridItem.command4)
                            } else {
                                resolutionSwitchBackPopup.open()
                            }
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
