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
    title: qsTr("Syna Video Player - SL1680")
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
        property string url2 : codecs_2.get(0).value
        property string url3 : codecs_3.get(0).value
        property string url4 : codecs_4.get(0).value
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

            Item {
                id: spacer_2
                height: spacer_1.height
                width: 1
                anchors.top: video_1.bottom
            }

            Button {
                id : video_2
                text: "VIDEO 2"
                font.pixelSize: 30
                width: video_1.width
                height: video_1.height
                highlighted : true
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: "#6495ed"
                }
                anchors.top: spacer_2.bottom
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                id: spacer_3
                height: spacer_1.height
                width: 1
                anchors.top: video_2.bottom
            }

            Button {
                id : video_3
                text: "VIDEO 3"
                font.pixelSize: 30
                width: video_1.width
                height: video_1.height
                highlighted : true
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: "#6495ed"
                }
                anchors.top: spacer_3.bottom
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                id: spacer_4
                height: spacer_1.height
                width: 1
                anchors.top: video_3.bottom
            }

            Button {
                id : video_4
                text: "VIDEO 4"
                font.pixelSize: 30
                width: video_1.width
                height: video_1.height
                highlighted : true
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: "#6495ed"
                }
                anchors.top: spacer_4.bottom
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

            Item {
                id: spacer_6
                height: spacer_1.height
                width: 1
                anchors.top: comboBox_1.bottom
            }

            ComboBox {
                id: comboBox_2
                currentIndex: 0
                textRole: "text"
                valueRole: "value"
                font.pixelSize: 30

                delegate: ItemDelegate {
                    width: comboBox_2.width
                    text: comboBox_2.textRole ? (Array.isArray(comboBox_2.model) ? modelData[comboBox_2.textRole] : model[comboBox_2.textRole]) : modelData
                    font.weight: comboBox_2.currentIndex === index ? Font.DemiBold : Font.Normal
                    font.family: comboBox_2.font.family
                    font.pointSize: comboBox_2.font.pointSize
                    highlighted: comboBox_2.highlightedIndex === index
                    hoverEnabled: comboBox_2.hoverEnabled
                }

                model: ListModel {
                    id: codecs_2
                    ListElement { text: "Starbucks"; value: "filesrc location=/home/root/demos/videos/Starbucks.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "I am Legend - Trailer"; value: "filesrc location=/home/root/demos/videos/IAmLegend.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "IKEA"; value: "filesrc location=/home/root/demos/videos/IKEA.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "CocaCola"; value: "filesrc location=/home/root/demos/videos/CocaCola.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "Camera Source"; value: "v4l2src device=CAM ! video/x-raw,framerate=30/1,format=YUY2,width=640,height=480 ! waylandsink"}
                }
                width: video_1.width
                height: video_1.height
                anchors.top: spacer_6.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                onCurrentIndexChanged: {
                    box.url2 = codecs_2.get(currentIndex).value
                }
            }


            Item {
                id: spacer_7
                height: spacer_1.height
                width: 1
                anchors.top: comboBox_2.bottom
            }

            ComboBox {
                id: comboBox_3
                currentIndex: 0
                textRole: "text"
                valueRole: "value"
                font.pixelSize: 30

                delegate: ItemDelegate {
                    width: comboBox_3.width
                    text: comboBox_3.textRole ? (Array.isArray(comboBox_3.model) ? modelData[comboBox_3.textRole] : model[comboBox_3.textRole]) : modelData
                    font.weight: comboBox_3.currentIndex === index ? Font.DemiBold : Font.Normal
                    font.family: comboBox_3.font.family
                    font.pointSize: comboBox_3.font.pointSize
                    highlighted: comboBox_3.highlightedIndex === index
                    hoverEnabled: comboBox_3.hoverEnabled
                }

                model: ListModel {
                    id: codecs_3
                    ListElement { text: "I am Legend - Trailer"; value: "filesrc location=/home/root/demos/videos/IAmLegend.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "IKEA"; value: "filesrc location=/home/root/demos/videos/IKEA.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "CocaCola"; value: "filesrc location=/home/root/demos/videos/CocaCola.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "Starbucks"; value: "filesrc location=/home/root/demos/videos/Starbucks.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                }
                width: video_1.width
                height: video_1.height
                anchors.top: spacer_7.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                onCurrentIndexChanged: {
                    box.url3 = codecs_3.get(currentIndex).value
                }
            }

            Item {
                id: spacer_8
                height: spacer_1.height
                width: 1
                anchors.top: comboBox_3.bottom
            }

            ComboBox {
                id: comboBox_4
                currentIndex: 0
                textRole: "text"
                valueRole: "value"
                font.pixelSize: 30

                delegate: ItemDelegate {
                    width: comboBox_4.width
                    text: comboBox_4.textRole ? (Array.isArray(comboBox_4.model) ? modelData[comboBox_4.textRole] : model[comboBox_4.textRole]) : modelData
                    font.weight: comboBox_4.currentIndex === index ? Font.DemiBold : Font.Normal
                    font.family: comboBox_4.font.family
                    font.pointSize: comboBox_4.font.pointSize
                    highlighted: comboBox_4.highlightedIndex === index
                    hoverEnabled: comboBox_4.hoverEnabled
                }

                model: ListModel {
                    id: codecs_4
                    ListElement { text: "IKEA"; value: "filesrc location=/home/root/demos/videos/IKEA.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "CocaCola"; value: "filesrc location=/home/root/demos/videos/CocaCola.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "Starbucks"; value: "filesrc location=/home/root/demos/videos/Starbucks.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                    ListElement { text: "I am Legend - Trailer"; value: "filesrc location=/home/root/demos/videos/IAmLegend.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! waylandsink"}
                }
                width: video_1.width
                height: video_1.height
                anchors.top: spacer_8.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                onCurrentIndexChanged: {
                    box.url4 = codecs_4.get(currentIndex).value
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

            RadioButton {
                id: quadView
                anchors.top: spacer_9.bottom
                text: "QUAD VIEW"
                font.pixelSize: 30
                anchors.right: parent.right
                onClicked: {
                    box.singleView = false
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
                    } else if (!box.multiView){
                        playbackControl.createManager()
                        playbackControl.doPlayback(2, box.url1, box.url2, box.url3, box.url4)
                    } else if (box.multiView){
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
