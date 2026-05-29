import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.3

ApplicationWindow {
    id: player
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: "FullScreen"
    title: qsTr("Torq Demo - SL261x")
    color: "#ffffff"

    property var cmdsModel: null
    property string url1: ""
    property string selectedCategory: ""
    property string selectedSource: ""
    property string selectedInput: ""

    function getPipeline() {
        if (!contentLoader.item) {
            return ""
        }

        if (selectedCategory === "Image Classification") {
            if (selectedSource === "NPU") {
                return contentLoader.item.imageClassificationNPU
            }
            if (selectedSource === "CPU") {
                return contentLoader.item.imageClassificationCPU
            }
            return ""
        }

        if (selectedCategory === "Object Detection") {
            if (selectedInput === "ISP") {
                if (selectedSource === "NPU") {
                    return contentLoader.item.objectDetectionISPNPU
                }
                if (selectedSource === "CPU") {
                    return contentLoader.item.objectDetectionISPCPU
                }
                return ""
            }
            if (selectedSource === "NPU") {
                return contentLoader.item.objectDetectionUSBNPU
            }
            if (selectedSource === "CPU") {
                return contentLoader.item.objectDetectionUSBCPU
            }
            return ""
        }

        if (selectedCategory === "Pose Estimation") {
            if (selectedInput === "ISP") {
                if (selectedSource === "NPU") {
                    return contentLoader.item.poseEstimationISPNPU
                }
                if (selectedSource === "CPU") {
                    return contentLoader.item.poseEstimationISPCPU
                }
                return ""
            }
            if (selectedSource === "NPU") {
                return contentLoader.item.poseEstimationUSBNPU
            }
            if (selectedSource === "CPU") {
                return contentLoader.item.poseEstimationUSBCPU
            }
            return ""
        }

        return ""
    }

    onVisibleChanged: {
        var wid = -1
        var targetScreen
        for (var i = 0; i < Qt.application.screens.length; i++) {
            if (wid < Qt.application.screens[i].width) {
                wid = Qt.application.screens[i].width
                targetScreen = Qt.application.screens[i]
            }
        }
        player.screen = targetScreen
    }

    header: ToolBar {
        id: headerBar
        width: parent.width
        height: 70
        background: Rectangle {
            color: "#007dc3"
        }
        Item {
            anchors.fill: parent
            Image {
                source: "qrc:/images/astra_icon.png"
                width: 80
                height: 80
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20
                fillMode: Image.PreserveAspectFit
            }
            Label {
                text: qsTr("TORQ DEMO")
                font.pixelSize: 45
                font.bold: true
                color: "white"
                anchors.centerIn: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Drawer {
        id: navDrawer
        width: 300
        height: parent.height
        edge: Qt.LeftEdge
        modal: true
        focus: true
        background: Rectangle {
            color: "#007dc3"
        }
        Rectangle {
            id: headerRect
            width: parent.width
            height: 70
            color: "#005b91"
            anchors.top: parent.top
            Text {
                text: "Select Demo"
                color: "white"
                font.pixelSize: 30
                font.bold: true
                anchors.centerIn: parent
            }
        }
        ListView {
            id: drawerListView
            width: parent.width
            anchors.top: headerRect.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0
            spacing: 0
            model: ListModel {
                ListElement { name: "Image Classification" }
                ListElement { name: "Object Detection" }
                ListElement { name: "Pose Estimation" }
            }
            property int expandedIndex: -1
            delegate: Item {
                id: parentItem
                width: parent ? parent.width : 300
                height: drawerListView.expandedIndex === index ? 48 + subOptions.count * 44 : 48
                Column {
                    width: parent.width
                    ItemDelegate {
                        width: parent.width
                        background: Rectangle {
                            color: drawerListView.expandedIndex === index ? "#005b91" : "#007dc3"
                        }
                        contentItem: Text {
                            text: name
                            color: "white"
                            font.pixelSize: 22
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        onClicked: {
                            if (name === "Image Classification" || name === "Object Detection" || name === "Pose Estimation") {
                                drawerListView.expandedIndex = drawerListView.expandedIndex === index ? -1 : index;
                            }
                        }
                        height: 48
                    }
                    // Sub menu for IC, OD
                    Repeater {
                        id: subOptions
                        model: drawerListView.expandedIndex === index && (name === "Image Classification" || name === "Object Detection" || name === "Pose Estimation") ? ["NPU", "CPU"] : []
                        delegate: ItemDelegate {
                            width: parent.width
                            background: Rectangle {
                                color: "#005b91"
                            }
                            contentItem: Text {
                                text: modelData
                                color: "white"
                                font.pixelSize: 18
                                verticalAlignment: Text.AlignVCenter
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 32
                            }
                            height: 44
                            onClicked: {
                                selectedCategory = name;
                                selectedSource = modelData;
                                if (name === "Object Detection" || name === "Pose Estimation") {
                                    selectedInput = "USB";
                                } else {
                                    selectedInput = "";
                                }
                                contentLoader.source = "file:///home/root/demos/qmls/source_selector.qml";
                                navDrawer.close();
                            }
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: contentLoader
        anchors.fill: parent
    }

    Image {
        id: playButton
        source: "qrc:/images/play.png"
        width: 80
        height: 80
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
        scale: playMouseArea.containsMouse ? 1.1 : 1.0
        Behavior on scale {
            NumberAnimation { duration: 200 }
        }
        MouseArea {
            id: playMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                if (contentLoader.item) {
                    var selectedPipeline = getPipeline()
                    if (selectedPipeline !== "") {
                        playbackControl.doPlayback(1, selectedPipeline, "", "", "")
                    }
                }
            }
            cursorShape: Qt.PointingHandCursor
        }
    }

    Image {
        id: synaIcon
        source: "qrc:/images/syna_icon.png"
        width: 300
        fillMode: Image.PreserveAspectFit
        anchors.right: parent.right
        anchors.bottom: inputPanel.visible ? inputPanel.top : playbackPanel.top
        anchors.rightMargin: 16
        anchors.bottomMargin: 16
    }

    Rectangle {
        id: inputPanel
        width: parent.width - 32
        height: 60
        color: "#313131"
        visible: selectedCategory === "Object Detection" || selectedCategory === "Pose Estimation"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: playbackPanel.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.bottomMargin: 8
        radius: 12
        border.color: "#222"
        border.width: 2

        Row {
            anchors.centerIn: parent
            spacing: 24

            ItemDelegate {
                width: 120
                height: 40
                background: Rectangle {
                    color: selectedInput === "USB" ? "#3faae7" : "#0c3046"
                    radius: 8
                }
                contentItem: Text {
                    text: "USB"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: selectedInput = "USB"
            }

            ItemDelegate {
                width: 120
                height: 40
                background: Rectangle {
                    color: selectedInput === "ISP" ? "#3faae7" : "#0c3046"
                    radius: 8
                }
                contentItem: Text {
                    text: "ISP"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: selectedInput = "ISP"
            }
        }
    }

    Rectangle {
        id: playbackPanel
        width: parent.width - 32
        height: 70
        color: "#313131"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        radius: 12
        border.color: "#222"
        border.width: 2
        Image {
            source: "qrc:/images/exit.png"
            width: 48
            height: 48
            fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 24
            MouseArea {
                anchors.fill: parent
                onClicked: Qt.quit()
                cursorShape: Qt.PointingHandCursor
            }
        }
    }

    Image {
        id: navButton
        source: "qrc:/images/menu.png"
        anchors.left: parent.left
        anchors.top: headerBar.bottom
        anchors.leftMargin: 16
        anchors.topMargin: 16
        width: 40
        height: 40
        fillMode: Image.PreserveAspectFit
        scale: navMouseArea.containsMouse ? 1.1 : 1.0
        Behavior on scale {
            NumberAnimation { duration: 200 }
        }
        MouseArea {
            id: navMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: navDrawer.open()
            cursorShape: Qt.PointingHandCursor
        }
    }
}
