import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import QtQuick.Window 2.1

import "../custom_views"

Rectangle {
    id: pe
    color: Resource.background

    property bool is1680: false

    Item {
        id: spacer_0
        height: Screen.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: pe.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isGray: true
            text: qsTr("Pose Estimation")
        }
    }

    GridLayout {
        id: gridLayout
        columns: 4
        columnSpacing: pe.width * 0.05
        rowSpacing: pe.width * 0.05
        width: pe.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 1
            mode: "file"
            image: "qrc:/res/images/file_icon.png"
            title:  qsTr("File")
            Layout.preferredWidth: gridLayout.width / 6
            Layout.preferredHeight: gridLayout.width / 6
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            modelPipeline: "tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=352,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=1920,height=1080,format=BGRA ! synapoverlay name=overlay ! placeholdersink"
        }

        GridItem {
            type: 1
            mode: "camera"
            image: "qrc:/res/images/camera_icon.png"
            title:  qsTr("USB Camera")
            Layout.preferredWidth: gridLayout.width / 6
            Layout.preferredHeight: gridLayout.width / 6
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            command1: "CAMSRC ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=352,format=RGB  ! synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA  ! synapoverlay name=overlay ! placeholdersink"
        }

        GridItem {
            id: cameraIsp1680
            type: 1
            mode: "camera"
            image: "qrc:/res/images/camera_icon.png"
            title: qsTr("ISP Camera")
            visible: is1680
            Layout.preferredWidth: gridLayout.width / 6
            Layout.preferredHeight: gridLayout.width / 6
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            command1: "CAMSRC name=ISP ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=352,format=RGB  ! synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA  ! synapoverlay name=overlay ! placeholdersink"
        }
    }
}
