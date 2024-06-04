import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import QtQuick.Window 2.1

import "../customviews"

Rectangle {
    id: pe
    color: Resource.background

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
        width: pe.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 1
            image: "qrc:/res/images/fileicon.png"
            title:  qsTr("File")
            command1: "filesrc location=/home/root/demos/videos/fitness.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! videoconvert !  tee name=t_data t_data. ! queue ! videoconvert ! videoscale ! video/x-raw,width=640,height=352,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synapoverlay name=overlay ! videoconvert ! waylandsink"
        }

        GridItem {
            type: 1
            image: "qrc:/res/images/cameraicon.png"
            title:  qsTr("Camera")
            command1: "v4l2src device=CAM ! video/x-raw,framerate=30/1,format=YUY2,width=640,height=480 ! videoconvert !  tee name=t_data t_data. ! queue ! videoconvert ! videoscale ! video/x-raw,width=640,height=352,format=RGB  ! synapinfer model=/usr/share/synap/models/object_detection/body_pose/model/yolov8s-pose/model.synap mode=detector frameinterval=3 ! overlay.inference_sink t_data. ! queue ! synapoverlay name=overlay ! videoconvert ! waylandsink"
        }
    }
}
