import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1
import Resource 1.0
import "./custom_views"
import "./panels"

ApplicationWindow {
    id: root
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: "FullScreen"
    title: qsTr("Syna AI Player - SL1680")
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
        Resource.width = targetScreen ? targetScreen.width : root.width
        Resource.height = targetScreen ? targetScreen.height : root.height
    }

    header: Header {
        id: headerView
    }

    Item {
        id: spacer_0
        height: Resource.width/15
        width: 1
        anchors.top: headerView.bottom
    }

    Component {
        id: objectDetection
        ObjectDetection {}
    }

    Component {
        id: poseEstimation
        PoseEstimation {}
    }

    Component {
        id: faceDetection
        FaceDetection {}
    }

    Component {
        id: multiAi
        MultiAi {}
    }

    Component {
        id: aIEncoding
        AIEncoding {}
    }

    Component {
        id: superres
        Superres {
            visible: !isX11
        }
    }

    Component {
        id: srslideshow
        SRSlideshow {
            visible: !isX11
        }
    }

    Component {
        id: facerecognition
        FaceRecognition {
            visible: !isX11
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
        case 4:
            sidemenu.sourceComponent = aIEncoding
            break;
        case 5:
            sidemenu.sourceComponent = isX11 ? null : superres
            break;
        case 6:
            sidemenu.sourceComponent = isX11 ? null : srslideshow
            break;
        case 7:
            sidemenu.sourceComponent = isX11 ? null : facerecognition
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
                    width: 10
                    policy: ScrollBar.AlwaysOn
                    background: Rectangle {
                        color: "#d6e9ff"
                        border.color: "#cccccc"
                        border.width: 1
                        radius: 5
                    }
                    contentItem: Rectangle {
                        color: Resource.baseColor
                        border.color: "#ffffff"
                        border.width: 1
                        radius: 5
                    }
                }
                id: listItemView
                anchors.fill: parent
                highlightFollowsCurrentItem: true
                spacing: Resource.width/36
                function updateModel() {
                    if (!isX11) {
                        var baseModel = ["Object Detection", "Pose Estimation", "Face Detection", "Multi-AI", "AI + Encoding", "Super Resolution", "SR Slideshow", "Face Recognition"]
                    }
                    else {
                        var baseModel = ["Object Detection", "Pose Estimation", "Face Detection", "Multi-AI", "AI + Encoding"]
                    }
                    return baseModel;
                }

                model: updateModel()
                delegate: ListItem {
                    spacing: Resource.width/36
                    title: modelData
                    width: listItemView.width
                    onClicked: {
                        ListView.view.currentIndex = index
                        root.switchPage(index)
                    }
                }
                Component.onCompleted: {
                    currentIndex = 0
                }
            }
        }
    }

    Item {
        width: Resource.rightPanelWidth
        height: Resource.rightPanelHeight
        anchors.top: headerView.bottom
        anchors.left: sidePanel.right
        StackView {
            id: pageStackView
            anchors.fill: parent
            initialItem: Loader {
                id: sidemenu
                sourceComponent: objectDetection
            }
        }
    }

    Button {
        id: exitBtn
        anchors.bottom: parent.bottom
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
