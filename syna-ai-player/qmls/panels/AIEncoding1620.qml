import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import QtQuick.Window 2.1

import "../custom_views"

Rectangle {
    id: od
    color: Resource.background

    Item {
        id: spacer_0
        height: Screen.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: od.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isGray: true
            text: qsTr("AI + Encoding")
        }
    }

    GridLayout {
        width: od.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 3
            mode: "camera"
            image: "qrc:/res/images/camera_icon.png"
            title:  qsTr("Camera")
            command1: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=256,height=160,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/model.synap mode=detector frameinterval=4 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! tee name=t_out t_out. ! queue ! synavideoconvertscale ! video/x-raw,format=NV12 ! x264enc speed-preset=1 ! rtph264pay ! udpsink host=HOST_IP port=8003 t_out. ! queue ! placeholdersink"
        }
    }
}
