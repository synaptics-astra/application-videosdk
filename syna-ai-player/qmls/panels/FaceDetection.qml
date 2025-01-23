import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import QtQuick.Window 2.1

import "../custom_views"

Rectangle {
    id: fd
    color: Resource.background

    Item {
        id: spacer_0
        height: Screen.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: fd.width
        spacing: 20
        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isGray: true
            text: qsTr("Face Detection")
        }
    }

    GridLayout {
        width: fd.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 1
            image: "qrc:/res/images/camera_icon.png"
            title:  qsTr("Camera")
            command1: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/face/model/yolov5s_face_640x480_onnx_mq/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay ! waylandsink"
        }
    }
}
