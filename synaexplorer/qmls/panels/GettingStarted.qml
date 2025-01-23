import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"

Rectangle {
    id: root
    color: Resource.background

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
                    playbackControl.doPlayback(1, "filesrc location=/home/root/demos/videos/mp4/astra_intro.mp4 ! qtdemux name=demux demux.audio_0 ! queue ! aacparse ! fdkaacdec ! audioconvert ! alsasink device=" + hwdevice + " demux.video_0 ! queue ! h264parse ! " + decoder + " ! waylandsink", "", "", "")
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
