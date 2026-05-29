import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Qt.labs.platform 1.1
import Qt.labs.folderlistmodel 2.15
import Resource 1.0

import "../custom_views"

Rectangle {
    id: root
    color: Resource.background

    property bool isFileSrcSelected: false
    property bool isCameraSrcSelected: true
    property bool isAIDemosSelected: false
    property bool isStreaming: false
    property string deviceIP: "0.0.0.0"
    property string selectedFilePath: ""
    property bool fileDialogOpen: false
    property string cameraPath: ""
    property url currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
    property bool isMachine1620: webrtcManager ? webrtcManager.getMachine().includes("1620") : false
    property bool isMachine2619: webrtcManager ? webrtcManager.getMachine().includes("2619") : false
    property bool isMachine2615: webrtcManager ? webrtcManager.getMachine().includes("2615") : false
    property bool isFaceDetectionError: false

    Component.onCompleted: {
        checkCameraDevices()
    }

    Popup {
        id: noInternetPopup
        anchors.centerIn: parent
        width: 400
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: "#ffffff"
            border.color: "#666666"
            border.width: 1
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 20
            anchors.fill: parent
            anchors.margins: 20

            Text {
                text: "No Internet Connection"
                font.pixelSize: 20
                font.bold: true
                color: "black"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Please check your internet connection and try again."
                font.pixelSize: 16
                color: "black"
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    noInternetPopup.close()
                    Qt.quit()
                }
                background: Rectangle {
                    radius: 8
                    color: "#003366"
                    border.color: "#002244"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    Item {
        id: spacer_0
        height: Resource.width/50
        width: 1
    }

    function checkCameraDevices() {
        cameraPath = webrtcManager.getCameraPath()
        statusText.text = cameraPath !== "" ? "Camera source found" : "No camera source found"
        return cameraPath !== ""
    }

    Connections {
        target: webrtcManager
        function onFileSrcEos() {
            if (root.isFileSrcSelected) {
                root.isStreaming = false
                sourceCombo.enabled = true
                statusText.text = "Streaming finished"
                root.deviceIP = "0.0.0.0"
                webrtcManager.stopServers()
            }
        }
        function onAiDemoSrcEos() {
            if (root.isAIDemosSelected) {
                root.isStreaming = false
                sourceCombo.enabled = true
                statusText.text = "Streaming finished"
                root.deviceIP = "0.0.0.0"
                webrtcManager.stopServers()
            }
        }
        function onInvalidFileSelected() {
            invalidFilePopup.open()
            root.selectedFilePath = ""
            if (root.isAIDemosSelected && aiDemoCombo.currentText === "Face Detection") {
                root.isFaceDetectionError = true
                statusText.text = "No camera source found"
                root.isAIDemosSelected = false
                aiDemosPopup.close()
            } else {
                root.isFaceDetectionError = false
                statusText.text = "Select a source to begin"
            }
        }
    }

    ColumnLayout {
        id: header
        width: parent.width
        spacing: 20
        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isHighlight: true
            text: qsTr("GStreamer WebRTC for Real-Time Communication")
        }
    }

    Popup {
        id: invalidFilePopup
        anchors.centerIn: parent
        width: 400
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#ffffff"
            border.color: "#666666"
            border.width: 1
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 20
            anchors.fill: parent
            anchors.margins: 20

            Text {
                text: root.isFaceDetectionError ? "No Camera Found" : "Invalid File"
                font.pixelSize: 20
                font.bold: true
                color: "black"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: root.isFaceDetectionError 
                      ? "No camera device was detected. Please connect a camera and try again." 
                      : "Only playable H.264 MP4 files are supported."
                font.pixelSize: 16
                color: "black"
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    invalidFilePopup.close()
                    root.isFaceDetectionError = false
                }
                background: Rectangle {
                    radius: 8
                    color: "#003366"
                    border.color: "#002244"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    Popup {
        id: filePopup
        anchors.centerIn: parent
        width: 600
        height: 400
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        visible: root.fileDialogOpen && root.isMachine1620

        background: Rectangle {
            color: "#ffffff"
            border.color: "#666666"
            border.width: 1
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 10

            Text {
                text: "Select a Video File"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
                color: "black"
            }

            TextField {
                id: pathField
                text: root.currentFolder.toString().replace("file://", "")
                Layout.fillWidth: true
                Layout.margins: 10
                readOnly: true
                font.pixelSize: 14
                background: Rectangle {
                    color: "#f0f0f0"
                    radius: 4
                    border.color: "#cccccc"
                    border.width: 1
                }
            }

            ListView {
                id: fileList
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 10
                clip: true
                ScrollBar.vertical: ScrollBar { active: true }
                model: FolderListModel {
                    id: folderModel
                    folder: root.currentFolder
                    nameFilters: ["*.mp4"]
                    showDirsFirst: true
                    showDotAndDotDot: false
                    showFiles: true
                    sortField: FolderListModel.Name
                }

                delegate: ItemDelegate {
                    width: fileList.width
                    height: 40
                    highlighted: ListView.isCurrentItem
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 10
                        Text {
                            text: model.fileName
                            font.pixelSize: 16
                            color: model.fileIsDir ? "#003366" : "black"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                    onClicked: {
                        if (model.fileIsDir) {
                            root.currentFolder = "file://" + model.filePath
                        } else {
                            let path = model.filePath.toString().replace("file://", "")
                            if (webrtcManager.validateFile(path)) {
                                root.selectedFilePath = path
                                statusText.text = "Selected file: " + model.fileName
                                root.fileDialogOpen = false
                                filePopup.close()
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Button {
                    text: "Back"
                    enabled: folderModel.parentFolder.toString() !== ""
                    onClicked: {
                        if (folderModel.parentFolder.toString() !== "") {
                            root.currentFolder = folderModel.parentFolder
                        }
                    }
                    background: Rectangle {
                        radius: 8
                        color: parent.enabled ? "#003366" : "#999999"
                        border.color: parent.enabled ? "#002244" : "#777777"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: "Cancel"
                    onClicked: {
                        statusText.text = "File selection canceled"
                        root.fileDialogOpen = false
                        root.selectedFilePath = ""
                        filePopup.close()
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#003366"
                        border.color: "#002244"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    Popup {
        id: aiDemosPopup
        anchors.centerIn: parent
        width: 300
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        visible: root.isAIDemosSelected

        background: Rectangle {
            color: "#ffffff"
            border.color: "#666666"
            border.width: 1
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 20

            Text {
                text: "Select AI Demo"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
                color: "black"
            }

            ComboBox {
                id: aiDemoCombo
                Layout.fillWidth: true
                model: ["Object Detection", "Pose Estimation", "Face Detection"]
                font.pixelSize: 16
                currentIndex: 0

                background: Rectangle {
                    radius: 8
                    color: "#ffffff"
                    border.color: "#666666"
                    border.width: 1
                }

                contentItem: Text {
                    text: aiDemoCombo.displayText
                    color: "black"
                    font.pixelSize: 16
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                }

                onActivated: {
                    if (root.isMachine1620 && !checkCameraDevices()) {
                        root.isFaceDetectionError = true
                        invalidFilePopup.open()
                        statusText.text = "No camera source found"
                        root.isAIDemosSelected = false
                        aiDemosPopup.close()
                        return
                    }
                    if (currentText === "Face Detection" && !root.isMachine1620) {
                        if (!checkCameraDevices()) {
                            root.isFaceDetectionError = true
                            invalidFilePopup.open()
                            statusText.text = "No camera source found"
                            root.isAIDemosSelected = false
                            aiDemosPopup.close()
                            return
                        }
                    }
                    root.isFaceDetectionError = false
                    statusText.text = "Selected AI Demo: " + currentText
                    root.isFileSrcSelected = false
                    root.isCameraSrcSelected = false
                    root.isAIDemosSelected = true
                    aiDemosPopup.close()
                }
            }

            Button {
                text: "Cancel"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    statusText.text = "AI Demo selection canceled"
                    root.isAIDemosSelected = false
                    root.isFaceDetectionError = false
                    aiDemosPopup.close()
                }
                background: Rectangle {
                    radius: 8
                    color: "#003366"
                    border.color: "#002244"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        TextView {
            text: "Select Source:"
            font.pixelSize: 26
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            ComboBox {
                id: sourceCombo
                Layout.preferredWidth: Resource.width / 4
                Layout.preferredHeight: 50
                model: root.isMachine2619 || root.isMachine2615 ? ["Camera", "FileSrc"] : ["Camera", "FileSrc", "AI Demos"]
                font.pixelSize: 18
                currentIndex: 0

                background: Rectangle {
                    radius: 8
                    color: sourceCombo.enabled ? "#ffffff" : "#dddddd"
                    border.color: sourceCombo.enabled ? "#666666" : "#999999"
                    border.width: 1
                }

                contentItem: Text {
                    text: sourceCombo.displayText
                    color: sourceCombo.enabled ? "black" : "#888888"
                    font.pixelSize: 18
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                }

                indicator: Canvas {
                    id: indicator
                    width: 14
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 10

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.beginPath()
                        ctx.moveTo(0, 0)
                        ctx.lineTo(width, 0)
                        ctx.lineTo(width / 2, height)
                        ctx.closePath()
                        ctx.fillStyle = sourceCombo.enabled ? "#333333" : "#aaaaaa"
                        ctx.fill()
                    }
                }

                delegate: ItemDelegate {
                    width: sourceCombo.width
                    height: 40
                    contentItem: Text {
                        text: modelData
                        font.pixelSize: 18
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                    }
                }

                onActivated: {
                    switch (currentText) {
                        case "AI Demos":
                            root.isFileSrcSelected = false
                            root.isCameraSrcSelected = false
                            root.isAIDemosSelected = true
                            root.isFaceDetectionError = false
                            aiDemosPopup.open()
                            break
                        case "FileSrc":
                            root.isFileSrcSelected = true
                            root.isCameraSrcSelected = false
                            root.isAIDemosSelected = false
                            root.isFaceDetectionError = false
                            root.fileDialogOpen = true
                            filePopup.open()
                            break
                        case "Camera":
                            root.isFileSrcSelected = false
                            root.isCameraSrcSelected = true
                            root.isAIDemosSelected = false
                            root.isFaceDetectionError = false
                            checkCameraDevices()
                            break
                        default:
                            statusText.text = "Unknown selection"
                            root.isFaceDetectionError = false
                    }
                }
            }
        }

        Text {
            id: statusText
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            wrapMode: Text.WordWrap
            text: "Select a source to begin"
            color: "black"
        }

        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: "Start"
                enabled: (root.isFileSrcSelected && root.selectedFilePath !== "") ||
                         (root.isCameraSrcSelected && root.cameraPath !== "") ||
                         (root.isAIDemosSelected && aiDemoCombo.currentText !== "")
                background: Rectangle {
                    radius: 8
                    color: parent.enabled ? "#003366" : "#999999"
                    border.color: parent.enabled ? "#002244" : "#777777"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (!webrtcManager.hasInternetConnection()) {
                        noInternetPopup.open()
                        return
                    }
                    if (!root.isStreaming) {
                        sourceCombo.enabled = false
                        webrtcManager.startServers()

                        if (root.isFileSrcSelected && root.selectedFilePath !== "") {
                            var fileName = root.selectedFilePath.substring(root.selectedFilePath.lastIndexOf('/') + 1)
                            webrtcManager.streamFileSrc(root.selectedFilePath)
                            root.deviceIP = webrtcManager.getEthernetIP()
                            root.isStreaming = true
                            statusText.text = "Streaming file: " + fileName
                        } else if (root.isCameraSrcSelected && root.cameraPath !== "") {
                            webrtcManager.streamCameraSrc(root.cameraPath)
                            root.deviceIP = webrtcManager.getEthernetIP()
                            root.isStreaming = true
                            statusText.text = "Streaming from camera: " + root.cameraPath
                        } else if (root.isAIDemosSelected && aiDemoCombo.currentText !== "") {
                            webrtcManager.streamAIDemos(aiDemoCombo.currentText)
                            root.deviceIP = webrtcManager.getEthernetIP()
                            root.isStreaming = true
                            statusText.text = "Streaming AI Demo: " + aiDemoCombo.currentText
                        }
                    }
                }
            }

            Button {
                text: "Stop"
                enabled: root.isStreaming
                background: Rectangle {
                    radius: 8
                    color: parent.enabled ? "#003366" : "#999999"
                    border.color: parent.enabled ? "#002244" : "#777777"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    webrtcManager.stopServers()
                    webrtcManager.stopFileSrc()
                    webrtcManager.stopCameraSrc()
                    webrtcManager.stopAIDemos()
                    sourceCombo.enabled = true
                    root.isStreaming = false
                    statusText.text = "Streaming stopped"
                    root.deviceIP = "0.0.0.0"
                    root.isFaceDetectionError = false
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: 20
            color: "transparent"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "transparent"

            Text {
                anchors.fill: parent
                font.pixelSize: 26
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                text: root.isStreaming ?
                      ("Open browser and watch your stream on <span style='color:green;'>" + root.deviceIP + ":8000</span>") :
                      ""
                color: root.isStreaming ? "blue" : "transparent"
            }
        }
    }
}