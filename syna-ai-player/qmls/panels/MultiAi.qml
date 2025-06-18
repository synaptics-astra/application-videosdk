import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import QtQuick.Window 2.1

import "../custom_views"

Rectangle {
    id: ma
    color: Resource.background

    property var rtspUrlsModel: null
    Component.onCompleted: {
        var rtspUrls = settingsManager.getRtspUrls()
        if (rtspUrls === null || rtspUrls.length === 0) {
            rtspUrlsModel = ["", "", "", ""]
        } else {
            rtspUrlsModel = rtspUrls
        }
    }

    Popup {
        id: dialog
        modal: true
        focus: true
        anchors.centerIn: ma.contentItem

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

                Repeater {
                    id: repeater
                    model: rtspUrlsModel
                    Layout.topMargin: 20
                    delegate: RowLayout {
                        id: rowLayout
                        spacing: 10
                        Layout.fillWidth: true

                        Text {
                            text: "RTSP URL " + (index + 1)
                            Layout.fillWidth: true
                        }

                        TextField {
                            text: modelData
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
            for (var i = 0; i < 4; i++) {
                var row = repeater.itemAt(i)
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
            }
            settingsManager.setRtspUrls(urls)
            settingsManager.saveConfig("/home/root/demos/configs/synaexplorer_config.txt")
        }
    }

    Item {
        id: spacer_0
        height: Screen.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: ma.width
        spacing: 20
        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isGray: true
            text: qsTr("Multi-AI")
        }
    }

    GridLayout {
        id: gridLayout
        width: ma.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            id: multiaicam
            type: 2
            image: "qrc:/res/images/multi_icon.png"
            title:  qsTr("USB Camera")
            command1: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=384,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"
            command2: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/face/model/yolov5s_face_640x480_onnx_mq/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay ! placeholdersink"
            command3: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=352,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay ! placeholdersink"
            command4: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=384,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"
        }

        GridItem {
            id: multiaicammixer
            type: 1
            mode: "camera"
            image: "qrc:/res/images/multi_icon.png"
            title:  qsTr("AI on muxed 9x480p")
                command1: "glvideomixerelement background=1 name=comp
                sink_0::xpos=0 sink_0::ypos=0 sink_0::width=640 sink_0::height=480
                sink_1::xpos=0 sink_1::ypos=500 sink_1::width=640 sink_1::height=480
                sink_2::xpos=0 sink_2::ypos=1000 sink_2::width=640 sink_2::height=480
                sink_3::xpos=660 sink_3::ypos=0 sink_3::width=640 sink_3::height=480
                sink_4::xpos=660 sink_4::ypos=500 sink_4::width=640 sink_4::height=480
                sink_5::xpos=660 sink_5::ypos=1000 sink_5::width=640 sink_5::height=480
                sink_6::xpos=1320 sink_6::ypos=0 sink_6::width=640 sink_6::height=480
                sink_7::xpos=1320 sink_7::ypos=500 sink_7::width=640 sink_7::height=480
                sink_8::xpos=1320 sink_8::ypos=1000 sink_8::width=640 sink_8::height=480
                ! video/x-raw\(memory:GLMemory\),width=1960,height=1480 ! tee name=data_g
                data_g. ! glcolorscale ! video/x-raw\(memory:GLMemory\),width=640,height=384 ! glcolorconvert ! video/x-raw\(memory:GLMemory\),format=RGB !
                gldownload ! queue !
                synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector threshold=0.4 frameinterval=3 numinference=30 !
                overlay.inference_sink
                data_g. ! glcolorconvert ! video/x-raw\(memory:GLMemory\),format=BGRA ! gldownload !
                synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! video/x-raw, format=BGRA ! tee name=t_out
                CAMSRC ! glupload ! glcolorconvert ! video/x-raw\(memory:GLMemory\), format=RGBA ! tee name=t_stream1
                CAMSRC ! glupload ! glcolorconvert ! video/x-raw\(memory:GLMemory\), format=RGBA ! tee name=t_stream2
                CAMSRC ! glupload ! glcolorconvert ! video/x-raw\(memory:GLMemory\), format=RGBA ! tee name=t_stream3
                CAMSRC ! glupload ! glcolorconvert ! video/x-raw\(memory:GLMemory\), format=RGBA ! tee name=t_stream4
                t_stream1. ! comp.sink_0
                t_stream2. ! comp.sink_1
                t_stream3. ! comp.sink_2
                t_stream4. ! comp.sink_3
                t_stream1. ! comp.sink_4
                t_stream2. ! comp.sink_5
                t_stream2. ! comp.sink_6
                t_stream3. ! comp.sink_7
                t_stream4. ! comp.sink_8
                t_out. ! queue ! placeholdersink"
        }

        GridItem {
            id: multiairtspcam
            type: 2
            image: "qrc:/res/images/multi_icon.png"
            title:  qsTr("RTSP Camera")
            command1: "rtspsrc location=RTSPURL latency=500 ! rtpjitterbuffer ! rtph264depay wait-for-keyframe=true ! video/x-h264, width=1920, height=1080 ! h264parse ! v4l2h264dec ! synavideoconvertscale ! video/x-raw, width=640, height=384, format=BGRA ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=384,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector frameinterval=4 threshold=0.3 ! overlay.inference_sink t_data. ! queue ! textoverlay name=title text=\"Object Detection\" valignment=top halignment=center font-desc=\"Sans, 14\" shaded-background=yes ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"
            command2: "rtspsrc location=RTSPURL latency=500 ! rtpjitterbuffer ! rtph264depay wait-for-keyframe=true ! video/x-h264, width=1920, height=1080 ! h264parse ! v4l2h264dec ! synavideoconvertscale ! video/x-raw, width=640, height=384, format=BGRA ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=384,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector frameinterval=4 threshold=0.3 ! overlay.inference_sink t_data. ! queue ! textoverlay name=title text=\"Object Detection\" valignment=top halignment=center font-desc=\"Sans, 14\" shaded-background=yes ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"
            command3: "rtspsrc location=RTSPURL latency=500 ! rtpjitterbuffer ! rtph264depay wait-for-keyframe=true ! video/x-h264, width=1920, height=1080 ! h264parse ! v4l2h264dec ! synavideoconvertscale ! video/x-raw, width=640, height=384, format=BGRA ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=384,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector frameinterval=4 threshold=0.3 ! overlay.inference_sink t_data. ! queue ! textoverlay name=title text=\"Object Detection\" valignment=top halignment=center font-desc=\"Sans, 14\" shaded-background=yes ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"
            command4: "rtspsrc location=RTSPURL latency=500 ! rtpjitterbuffer ! rtph264depay wait-for-keyframe=true ! video/x-h264, width=1920, height=1080 ! h264parse ! v4l2h264dec ! synavideoconvertscale ! video/x-raw, width=640, height=384, format=BGRA ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=384,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap mode=detector frameinterval=4 threshold=0.3 ! overlay.inference_sink t_data. ! queue ! textoverlay name=title text=\"Object Detection\" valignment=top halignment=center font-desc=\"Sans, 14\" shaded-background=yes ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"
        }
    }

    RowLayout {
        anchors.top: gridLayout.bottom
        anchors.right: gridLayout.right
        Button {
            id: update
            width: ma.width * 0.9
            text: "Update RTSP Urls"
            font.pixelSize: 25
            background: Rectangle {
                width: parent.width
                radius: 20
                border.color: Resource.baseColor
            }

            onClicked: {
                dialog.open()
            }
        }
    }
}
