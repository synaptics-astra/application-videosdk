import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"

Pane {
    id: card
    property int type: 0
    property color lineColor: "#007dc3"
    property string image: ""
    property string title: ""
    padding: 0
    width: Resource.rightPanelWidth/4
    height: Resource.rightPanelWidth/4

    property real rootWidth: Window.window ? Window.window.width : Resource.width
    property real rootHeight: Window.window ? Window.window.height : Resource.height
    property string synaDrmFilePath: ""

    Component.onCompleted: {
        synaDrmFilePath = processHelper.findFile(["/etc/modprobe.d/syna_drm.conf", "/usr/lib/modprobe.d/syna_drm.conf"])
        if (!synaDrmFilePath) {
            console.log("Error: syna_drm.conf not found in either location")
        }
    }

    background: Rectangle {
        anchors.fill: parent
        color: palette.light
        radius: 15

        Rectangle {
            anchors.bottom: parent.bottom
            height: 10
            width: Resource.rightPanelWidth/4
            radius: 10
            color: card.lineColor
        }
    }

    Popup {
        id: resolutionSwitchBackPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Math.max(0.20 * card.rootWidth, 250)
        height: Math.max(0.15 * card.rootHeight, 120)

        background: Rectangle {
            color: "#f0f0f0"
            radius: card.rootWidth / 240
            border.color: "#ccc"
        }

        contentItem: ColumnLayout {
            spacing: card.rootWidth / 120
            anchors.fill: parent
            anchors.margins: card.rootWidth / 120

            Text {
                text: "To run this application, we need to switch to 2K UI. Press OK to continue."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: card.rootWidth / 90
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: card.rootWidth / 120

                Button {
                    text: "Ok"
                    Layout.fillWidth: true
                    contentItem: Text {
                        text: parent.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: card.rootWidth / 110
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
                        font.pixelSize: card.rootWidth / 110
                    }
                    onClicked: {
                        resolutionSwitchBackPopup.close()
                    }
                }
            }
        }
    }

    contentItem: Item {
        anchors.fill: parent
        MouseArea {
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

                if (card.type === 5) {
                    playbackControl.launch("/usr/bin/syna-ai-player")
                } else if (is2K) {
                    switch (card.type) {
                        case 0:
                            playbackControl.launch("/usr/bin/syna-video-player")
                            break;
                        case 1:
                            playbackControl.doPlayback(1, "multifilesrc location=/home/root/demos/videos/h264/sample_1_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_0 multifilesrc location=/home/root/demos/videos/h264/sample_2_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_1 multifilesrc location=/home/root/demos/videos/h264/sample_3_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_2 multifilesrc location=/home/root/demos/videos/h264/sample_4_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_3 multifilesrc location=/home/root/demos/videos/h264/sample_5_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_4 multifilesrc location=/home/root/demos/videos/h264/sample_6_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_5 multifilesrc location=/home/root/demos/videos/h264/sample_7_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_6 multifilesrc location=/home/root/demos/videos/h264/sample_8_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_7 multifilesrc location=/home/root/demos/videos/h264/sample_9_720p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_8 glvideomixer name=comp sink_0::xpos=0 sink_0::ypos=0 sink_0::width=640 sink_0::height=360 sink_1::xpos=0 sink_1::ypos=360 sink_1::width=640 sink_1::height=360 sink_2::xpos=0 sink_2::ypos=720 sink_2::width=640 sink_2::height=360 sink_3::xpos=640 sink_3::ypos=0 sink_3::width=640 sink_3::height=360 sink_4::xpos=640 sink_4::ypos=360 sink_4::width=640 sink_4::height=360 sink_5::xpos=640 sink_5::ypos=720 sink_5::width=640 sink_5::height=360 sink_6::xpos=1280 sink_6::ypos=0 sink_6::width=640 sink_6::height=360 sink_7::xpos=1280 sink_7::ypos=360 sink_7::width=640 sink_7::height=360 sink_8::xpos=1280 sink_8::ypos=720 sink_8::width=640 sink_8::height=360 ! video/x-raw, format=NV12 ! queue2 ! v4l2convert ! placeholdersink", "", "", "")
                            break;
                        case 2:
                            playbackControl.launch("/usr/share/examples/opengl/textures/textures")
                            break;
                        case 3:
                            playbackControl.launch("/usr/share/examples/widgets/animation/animatedtiles/animatedtiles")
                            break;
                        case 4:
                            playbackControl.launch("/usr/share/examples/widgets/touch/fingerpaint/fingerpaint")
                            break;
                        case 6:
                            playbackControl.doPlayback(1, "multifilesrc location=/home/root/demos/videos/h264/sample_1_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_0 multifilesrc location=/home/root/demos/videos/h264/sample_2_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_1 multifilesrc location=/home/root/demos/videos/h264/sample_3_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_2 multifilesrc location=/home/root/demos/videos/h264/sample_4_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_3 multifilesrc location=/home/root/demos/videos/h264/sample_5_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_4 multifilesrc location=/home/root/demos/videos/h264/sample_6_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_5 multifilesrc location=/home/root/demos/videos/h264/sample_7_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_6 multifilesrc location=/home/root/demos/videos/h264/sample_8_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_7 multifilesrc location=/home/root/demos/videos/h264/sample_9_480p.h264 loop=true ! h264parse ! v4l2h264dec ! comp.sink_8 glvideomixer name=comp sink_0::xpos=0 sink_0::ypos=0 sink_0::width=640 sink_0::height=360 sink_1::xpos=0 sink_1::ypos=360 sink_1::width=640 sink_1::height=360 sink_2::xpos=0 sink_2::ypos=720 sink_2::width=640 sink_2::height=360 sink_3::xpos=640 sink_3::ypos=0 sink_3::width=640 sink_3::height=360 sink_4::xpos=640 sink_4::ypos=360 sink_4::width=640 sink_4::height=360 sink_5::xpos=640 sink_5::ypos=720 sink_5::width=640 sink_5::height=360 sink_6::xpos=1280 sink_6::ypos=0 sink_6::width=640 sink_6::height=360 sink_7::xpos=1280 sink_7::ypos=360 sink_7::width=640 sink_7::height=360 sink_8::xpos=1280 sink_8::ypos=720 sink_8::width=640 sink_8::height=360 ! video/x-raw, format=NV12 ! placeholdersink", "", "", "")
                            break;
                        case 7:
                            playbackControl.launch("/usr/bin/syna-capability-demo")
                            break;
                        case 8:
                            playbackControl.launch("/usr/bin/camera")
                            break;
                        case 9:
                            playbackControl.launch("/usr/bin/audio-classification")
                            break;
                        case 10:
                            playbackControl.launch("/usr/bin/syna-gst-webrtc")
                            break;
                        case 11:
                            playbackControl.launch("/usr/bin/syna-dash-player")
                            break;
                        case 12:
                            playbackControl.launch("/usr/bin/torq-demo")
                            break;
                        default:
                            break;
                    }
                } else {
                    resolutionSwitchBackPopup.open()
                }
            }
        }

        Rectangle {
            id: notifIcon
            width: card.width * 0.6
            height: card.height * 0.6
            color: palette.light
            anchors.centerIn: parent

            Image {
                anchors.centerIn: parent
                source: card.image
                sourceSize: Qt.size(notifIcon.width * 0.8, notifIcon.height * 0.8)
            }
        }
        ColumnLayout {
            width: card.width
            spacing: 10
            anchors.bottom: notifIcon.bottom
            TextView {
                isHighlight: true
                text: card.title
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
}
