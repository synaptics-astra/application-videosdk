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
                text: "To run this application, we need to switch to 2k UI. Press OK to continue."
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
                    var hwdevice;
                    var decoder;
                    var machine = playbackControl.getMachine();
                    if (machine == "sl1620") {
                        hwdevice = "hw:0,1";
                        decoder = "avdec_h264";
                    }
                    if (machine == "sl1640") {
                        hwdevice = "hw:0,8";
                        decoder = "v4l2h264dec";
                    }
                    if (machine == "sl1680") {
                        hwdevice = "hw:0,7";
                        decoder = "v4l2h264dec";
                    }
                    if (!processHelper.fileExists("/etc/modprobe.d/syna_drm.conf")) {
                        resolutionSwitchBackPopup.open()
                    } else {
                        playbackControl.doPlayback(1, "filesrc location=/home/root/demos/videos/mp4/astra_intro.mp4 ! qtdemux name=demux demux.audio_0 ! queue ! aacparse ! fdkaacdec ! audioconvert ! alsasink device=" + hwdevice + " demux.video_0 ! queue ! h264parse ! " + decoder + " ! placeholdersink", "", "", "")

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
