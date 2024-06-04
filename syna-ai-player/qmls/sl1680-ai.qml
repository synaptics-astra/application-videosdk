import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1

import Resource 1.0

import "./customviews"
import "./panels"

ApplicationWindow {
    id: root
    width: 1920
    height: 1080
    visible: true
    visibility: "FullScreen"
    title: qsTr("Syna AI Player - SL1680")
    color: Resource.background

    header: Header {
        id : headerView
    }

    Item {
        id: spacer_0
        height: Screen.width/15
        width: 1
        anchors.top: headerView.bottom
    }

    Component{
        id: objectDetection
        ObjectDetection {
        }
    }

    Component{
        id:poseEstimation
        PoseEstimation {
        }
    }

    Component{
        id:faceDetection
        FaceDetection {
        }
    }

    Component{
        id:multiAi
        MultiAi {
        }
    }


    function switchPage(index) {
        switch(index) {
        case 0:
            sidemenu.sourceComponent = objectDetection
            break;
        case 1:
            sidemenu.sourceComponent = poseEstimation
            break;
        case 2:
            sidemenu.sourceComponent = faceDetection
            break;
        case 3:
            sidemenu.sourceComponent = multiAi
            break;
        }
    }

    RowLayout {
        id: sidePanel
        anchors.top: spacer_0.bottom
        height: parent.height - headerView.height - spacer_0.height

        Pane {
            padding: 0
            Layout.preferredWidth: Resource.leftPanelWidth
            Layout.fillHeight: true
            background: Rectangle {
                anchors.fill: parent
                color: Resource.background
                Image {
                    source: "qrc:/res/images/verticalline.png"
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            ListView {
                id:listItemView
                anchors.fill: parent
                highlightFollowsCurrentItem :true
                spacing: Screen.width/36
                model:["Object Detection","Pose Estimation","Face Detection", "Multi-AI"]
                delegate:ListItem {
                    spacing: Screen.width/36
                    title: modelData
                    width: listItemView.width
                    onClicked: {
                        ListView.view.currentIndex = index
                        root.switchPage(index)
                    }
                }
            }
        }
    }

    Item {
        width : Resource.rightPanelWidth
        height : Resource.rightPanelHeight
        anchors.top: header.bottom
        anchors.left: sidePanel.right
        StackView {
            id:pageStackView
            anchors.fill: parent
            initialItem:Loader {
                id:sidemenu
                sourceComponent: objectDetection
            }
        }
    }

   Button {
       id: exitBtn
       anchors.bottom : parent.bottom
       opacity: 0.87
       text: "Exit"
       width: listItemView.width
       contentItem: Text {
           text: parent.text
           color: Resource.baseColor
           font.pixelSize: Resource.h4
           horizontalAlignment: Text.AlignHCenter
           verticalAlignment: Text.AlignVCenter
       }
       bottomPadding: Screen.width/45
       onClicked: {
           Qt.quit()
       }
       background: Rectangle {
           color: "transparent"
           anchors.fill: parent
           radius: 8
       }
   }

   Image {
       id: syna_logo
       source: "qrc:/res/images/synaicon.png"
       sourceSize: Qt.size(Resource.rightPanelWidth/4, Resource.rightPanelWidth/4)
       anchors.right: parent.right
       anchors.bottom: parent.bottom
   }
}
