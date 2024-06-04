import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

ApplicationWindow {
    width: 1920
    height: 1080
    visible: true
    visibility: "FullScreen"
    title: qsTr("Syna Video Player - SL1620")
    color: "#e6e6fa"

    ToolBar {
        id: overlayHeader
        z: 1
        width: parent.width
        height: 80
        Rectangle {
            width: parent.width
            height: parent.height
            color: "black"
            Image {
                source: "qrc:/images/synaptics_logo.jpg"
                width: 70
                height: 60
                x: 10
                y: 10
                Rectangle {
                    width: parent.width
                    height: parent.height
                    color: "transparent"
                }
            }

            Label {
                id: label
                anchors.centerIn: parent
                text: "Astra Video Playback Demo"
                font.pixelSize: 40
                font.bold: true
                color: "white"
            }
        }
    }

    Rectangle {
        id : box
        x: 50
        width: parent.width - 100
        height: parent.height - overlayHeader.height
        anchors.top: overlayHeader.bottom
        color: "#e6e6fa"

        property string url1 : codecs_1.get(0).value
        property bool singleView : true
        property bool multiView : false

        Label {
            id: selection
            text: "Please select the videos ..."
            font.pixelSize: 30
            font.bold: true
            topPadding: 20
        }

        Rectangle {
            id: leftArea
            width: parent.width/2
            height: parent.height/2
            anchors.top: selection.bottom
            color: "#e6e6fa"

            Item {
                id: spacer_1
                height: parent.width*0.01
                width: 1
            }

            Button {
                id : video_1
                text: qsTr("VIDEO 1")
                font.pixelSize: 30
                width: parent.width - parent.width*0.05
                height: 80
                highlighted : true
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: "#6495ed"
                }
                anchors.top: spacer_1.bottom
                anchors.horizontalCenter: parent.horizontalCenter
            }

        }

        Rectangle {
            id: rightArea
            width: parent.width/2
            height: parent.height/2
            anchors.left: leftArea.right
            anchors.top: selection.bottom
            color: "#e6e6fa"

            Item {
                id: spacer_5
                height: spacer_1.height
                width: 1
            }

            ComboBox {
                id: comboBox_1
                currentIndex: 0
                textRole: "text"
                valueRole: "value"
                font.pixelSize: 30

                delegate: ItemDelegate {
                    width: comboBox_1.width
                    text: comboBox_1.textRole ? (Array.isArray(comboBox_1.model) ? modelData[comboBox_1.textRole] : model[comboBox_1.textRole]) : modelData
                    font.weight: comboBox_1.currentIndex === index ? Font.DemiBold : Font.Normal
                    font.family: comboBox_1.font.family
                    font.pointSize: comboBox_1.font.pointSize
                    highlighted: comboBox_1.highlightedIndex === index
                    hoverEnabled: comboBox_1.hoverEnabled
                }

                model: ListModel {
                    id: codecs_1
                    ListElement { text: "CocaCola"; value: "filesrc location=/home/root/demos/videos/CocaCola.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "Starbucks"; value: "filesrc location=/home/root/demos/videos/Starbucks.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "I am Legend - Trailer"; value: "filesrc location=/home/root/demos/videos/IAmLegend.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "IKEA"; value: "filesrc location=/home/root/demos/videos/IKEA.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "Camera Source"; value: "v4l2src device=CAM ! video/x-raw,framerate=30/1,format=YUY2,width=1920,height=1080 ! waylandsink"}
                }
                Text {
                    font.pointSize: 80
                    font.bold: true
                }
                width: video_1.width
                height: video_1.height
                anchors.top: spacer_5.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                onCurrentIndexChanged: {
                    box.url1 = codecs_1.get(currentIndex).value
                }
            }
        }

        Label {
            id: option
            text: "Please select playback type"
            font.pixelSize: 30
            anchors.bottom: bottomArea.bottom
            anchors.left: bottomArea.left
        }

        Rectangle {
            id: bottomArea
            width: parent.width/2
            height: parent.height/2
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#e6e6fa"

            Item {
                id: spacer_9
                height: spacer_1.height
                width: 1
            }

            RadioButton {
                id: singleView
                checked: true
                anchors.top: spacer_9.bottom
                text: "SINGLE VIEW"
                font.pixelSize: 30
                onClicked: {
                    box.singleView = true
                    box.multiView = false
                }
            }

            Item {
                id: spacer_0
                height: 50
                width: 1
                anchors.top: singleView.bottom
            }

            Button {
                id : play
                text: "PLAY"
                font.pixelSize: 30
                width: parent.width
                height: video_1.height
                highlighted : true
                anchors.top: spacer_0.bottom
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: play.down ? "#000000" : "#a9a9a9"
                }
                onClicked: {
                    if (box.singleView && !box.multiView) {
                        playbackControl.createManager()
                        playbackControl.doPlayback(1, box.url1, "", "", "")
                    }
                }
            }

            Item {
                id: spacer_00
                height: 50
                width: 1
                anchors.top: play.bottom
            }

            Button {
                id : exit
                text: "Exit"
                font.pixelSize: 30
                width: parent.width
                height: video_1.height
                highlighted : true
                anchors.top: spacer_00.bottom
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: exit.down ? "#000000" : "#a9a9a9"
                }
                onClicked: {
                    Qt.quit()
                }
            }
        }

    }
}
