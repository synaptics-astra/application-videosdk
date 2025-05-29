import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1

import Resource 1.0

import "./custom_views"
import "./panels"

ApplicationWindow {
    id: root
    width: 1920
    height: 1080
    visible: true
    visibility: "FullScreen"
    title: qsTr("Syna AI Player - SL1640")
    color: Resource.background

    onVisibleChanged: {
        var wid = -1
        var targetScreen
        for (var i = 0; i < Qt.application.screens.length; i++) {
            if (wid < Qt.application.screens[i].width) {
                wid = Qt.application.screens[i].width
                targetScreen = Qt.application.screens[i]
            }
        }
        this.screen = targetScreen
        Resource.width = targetScreen.width
        Resource.height = targetScreen.height
    }

    header: Header {
        id : headerView
    }

    Item {
        id: spacer_0
        height: Resource.width/15
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
        id:aIEncoding
        AIEncoding {
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
            sidemenu.sourceComponent = aIEncoding
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
            }

            ListView {
                ScrollBar.vertical: ScrollBar {
                    id: scrollBar
                    width: 10
                    policy: ScrollBar.AlwaysOn
                    background: Rectangle {
                        color: "#d6e9ff" // background color
                        border.color: "#cccccc" // border color
                        border.width: 1
                        radius: 5
                    }
                    contentItem: Rectangle {
                        color: "#3d88f0" // handle color
                        border.color: "#ffffff" // handle border color
                        border.width: 1
                        radius: 5
                    }
                }

                id:listItemView
                anchors.fill: parent
                highlightFollowsCurrentItem :true
                spacing: Resource.width/36
                model:["Object Detection","Pose Estimation","Face Detection", "AI + Encoding"]
                delegate:ListItem {
                    spacing: Resource.width/36
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
       bottomPadding: Resource.width/45
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
       source: "qrc:/res/images/syna_icon.png"
       sourceSize: Qt.size(Resource.rightPanelWidth/4, Resource.rightPanelWidth/4)
       anchors.right: parent.right
       anchors.bottom: parent.bottom
   }
}
