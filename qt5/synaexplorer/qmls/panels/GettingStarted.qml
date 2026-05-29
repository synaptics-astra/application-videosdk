import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"

Rectangle {
    id: root
    color: Resource.background
    property real rootWidth: Window.window ? Window.window.width : Resource.width
    property real rootHeight: Window.window ? Window.window.height : Resource.height

    Component.onCompleted: {
        synaDrmFilePath = processHelper.findFile(["/etc/modprobe.d/syna_drm.conf", "/usr/lib/modprobe.d/syna_drm.conf"])
        if (!synaDrmFilePath) {
            console.log("Error: syna_drm.conf not found in either location")
        }
    }

    property string synaDrmFilePath: ""

    Popup {
        id: resolutionSwitchBackPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.max(0.20 * root.rootWidth, 250)
        height: Math.max(0.15 * root.rootHeight, 120)

        background: Rectangle {
            color: "#f0f0f0"
            radius: root.rootWidth / 240
            border.color: "#ccc"
        }

        contentItem: ColumnLayout {
            spacing: root.rootWidth / 120
            anchors.fill: parent
            anchors.margins: rootWidth / 120

            Text {
                text: "To run this application, we need to switch to 2K UI. Press OK to continue."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: rootWidth / 90
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: root.rootWidth / 120

                Button {
                    text: "Ok"
                    Layout.fillWidth: true
                    contentItem: Text {
                        text: parent.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: root.rootWidth / 110
                    }
                    onClicked: {
                        if (synaDrmFilePath) {
                            var contents = processHelper.readFile(synaDrmFilePath)
                            var updatedContents = contents
                            if (contents.includes("hdmi_preferred_mode=")) {
                                updatedContents = contents.replace("hdmi_preferred_mode=3840x2160", "hdmi_preferred_mode=1920x1080")
                            } else {
                                updatedContents = contents + (contents.endsWith("\n") ? "" : "\n") + "hdmi_preferred_mode=1920x1080"
                            }
                            if (processHelper.writeFile(synaDrmFilePath, updatedContents)) {
                                processHelper.executeCommand("sync && reboot")
                            } else {
                                console.log("Error: Failed to write to syna_drm.conf")
                            }
                        } else {
                            console.log("Error: Cannot switch to 2K, syna_drm.conf not found")
                        }
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
                        font.pixelSize: root.rootWidth / 110
                    }
                    onClicked: {
                        resolutionSwitchBackPopup.close()
                    }
                }
            }
        }
    }


    ColumnLayout {
        width: root.width
        height: root.height

        Image {
            source: "qrc:/res/images/astra_playback.png"
            scale: {
                    if (Resource.width > 1920) {
                        1.2
                    } else {
                        0.6
                    }
            }
            Layout.alignment: Qt.AlignHCenter
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    var is2K = false
                    if (synaDrmFilePath) {
                        var contents = processHelper.readFile(synaDrmFilePath)
                        if (contents.includes("hdmi_preferred_mode=1920x1080")) {
                            is2K = true
                        }
                    } else {
                        console.log("Error: syna_drm.conf not found, assuming 4K")
                    }

                    var hwdevice;
                    var decoder;
                    var machine = playbackControl.getMachine();
                    if (machine == "sl1620") {
                        decoder = "avdec_h264";
                    }
                    if (machine == "sl2619") {
                        decoder = "avdec_h264";
                    }
                    if (machine == "sl2615") {
                        decoder = "avdec_h264";
                    }
                    if (machine == "sl1640") {
                        decoder = "v4l2h264dec";
                    }
                    if (machine == "sl1680") {
                        decoder = "v4l2h264dec";
                    }
                    if (is2K) {
                        hwdevice = AudioHelper.getPriorityDeviceId()
                        playbackControl.doPlayback(1, "filesrc location=/home/root/demos/videos/mp4/astra_intro.mp4 ! qtdemux name=demux demux.audio_0 ! queue ! aacparse ! fdkaacdec ! audioconvert ! alsasink device=" + hwdevice + " demux.video_0 ! queue ! h264parse ! " + decoder + " ! placeholdersink", "", "", "")
                    } else {
                        resolutionSwitchBackPopup.open()
                    }
                }
            }
        }

        TextView {
            font.pixelSize: Resource.h2
            font.family: Resource.nasa21.name
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("AI-Native IoT Compute")
        }

        TextView {
            font.pixelSize: Resource.paragraph
            font.family: Resource.cabal.name
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("The Astra Platform has been architected to address the needs of customers building a new generation of smart IoT devices.")
        }
    }
}
