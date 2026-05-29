import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1
import Qt.labs.folderlistmodel 2.15

ApplicationWindow {
    id: root
    width: 3840
    height: 2160
    visible: true
    visibility: "FullScreen"
    title: qsTr("SR Slideshow")
    color: "#0069d9"

    property var leftImageSource
    property var rightImageSource
    property string invalidFilesText: ""
    property bool isClosing: false
    property bool isTransitioning: false
    property bool isPlaying: false
    property string userFilesDir: "file:///home/root/demos/user-slideshow-images"
    property bool isCameraActive: false
    property bool isCapturing: false
    property string lastCapturedImagePath: ""

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"
        z: -2
    }

    FolderListModel {
        id: userFilesModel
        folder: userFilesDir
        nameFilters: ["*.png"]
        showDirs: false
    }

    function updateImages() {
        if (isClosing) {
            return
        }
        var timestamp = new Date().getTime()
        leftImage.source  = "image://frameProvider/normalFrame?t=" + timestamp
        rightImage.source = "image://frameProvider/srFrame?t=" + timestamp
    }

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: header
            color: "#0069d9"
            Layout.fillWidth: true
            Layout.preferredHeight: root.height * 0.035
            Layout.topMargin: root.width * 0.005
            Layout.leftMargin: root.width * 0.005
            Layout.rightMargin: root.width * 0.005

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: root.width * 0.01
                anchors.rightMargin: root.width * 0.01

                Image {
                    id: headerIcon
                    source: "qrc:/res/images/astra_icon.png"
                    Layout.preferredHeight: root.height * 0.030
                    Layout.preferredWidth: Layout.preferredHeight
                    fillMode: Image.PreserveAspectFit
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    id: headerText
                    text: "SR Slideshow"
                    color: "white"
                    font.pixelSize: root.height * 0.0145
                    font.bold: true
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                }
            }
        }

        RowLayout {
            id: rowLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: -0.2
            spacing: 0

            Rectangle {
                id: playbackRect
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: root.width * 0.005
                Layout.alignment: Qt.AlignTop
                color: "white"
                border.width: 0
                border.color: "transparent"

                Image {
                    id: backgroundImage
                    anchors.fill: parent
                    source: "qrc:/res/images/sr.png"
                    fillMode: Image.PreserveAspectFit
                    visible: !isCameraActive
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Image {
                        id: leftImage
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceSize.width: root.width / 4
                        sourceSize.height: root.height / 2
                        cache: false
                    }

                    Image {
                        id: rightImage
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceSize.width: root.width / 4
                        sourceSize.height: root.height / 2
                        cache: false
                    }

                    Image {
                        id: cameraPreview
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceSize.width: 640
                        sourceSize.height: 480
                        cache: false
                        visible: isCameraActive
                        fillMode: Image.PreserveAspectFit
                        source: "image://cameraProvider/preview"

                       Connections {
                            target: cameraManager
                            function onPreviewUpdated() {
                                cameraPreview.source = "image://cameraProvider/preview?" + Math.random()
                            }
                            function onNoCameraDetected(message) {
                                noCameraPopup.open()
                            }
                            function onPreviewStarted() {
                                isCameraActive = true
                                navPanel.visible = false
                                isTransitioning = false
                            }
                        }

                        Button {
                            id: captureImageButton
                            width: root.height * 0.05
                            height: root.height * 0.05
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: root.height * 0.02
                            visible: isCameraActive
                            enabled: !isCapturing
                            hoverEnabled: false

                            background: Rectangle {
                                id: outerBorder
                                anchors.fill: parent
                                color: "transparent"
                                radius: width / 2
                                border.color: "#ffffff"
                                border.width: captureImageButton.pressed ? 8 : 6

                                Behavior on border.width { NumberAnimation { duration: 200 } }

                                Rectangle {
                                    id: innerButton
                                    width: parent.width
                                    height: parent.height
                                    anchors.centerIn: parent
                                    color: "transparent"
                                    radius: width / 2
                                    border.color: "#ffffff"
                                    border.width: 1
                                    opacity: isCapturing ? 0.5 : 1.0

                                    scale: captureImageButton.pressed ? 1.05 : 1.0
                                    Behavior on scale { NumberAnimation { duration: 200 } }
                                }
                            }

                            scale: pressed ? 0.9 : 1.0
                            Behavior on scale { NumberAnimation { duration: 100 } }

                            onClicked: {
                                isCapturing = true
                                cameraManager.captureImage()
                            }
                        }
                    }
                }

                Button {
                    id: playButton
                    width: root.height * 0.04 * 1.6 + 10
                    height: root.height * 0.04 * 1.6
                    anchors.centerIn: parent
                    z: 1
                    visible: leftImage.source.toString() === "" && rightImage.source.toString() === "" && !isCameraActive
                    opacity: visible ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 200 } }

                    background: Rectangle {
                        color: "#ccffffff"
                        radius: 8
                    }

                    contentItem: Item {
                        width: playButton.height * 1.6 + 10
                        height: playButton.height * 1.6
                        Image {
                            id: playIcon
                            source: "qrc:/res/images/play.png"
                            sourceSize.width: playButton.height
                            sourceSize.height: playButton.height
                            width: playButton.height
                            height: playButton.height
                            fillMode: Image.PreserveAspectFit
                            anchors.centerIn: parent
                        }
                    }

                    hoverEnabled: true
                    scale: hovered ? 1.1 : 1.0
                    Behavior on scale { NumberAnimation { duration: 200 } }
                    onClicked: {
                        if (frameProvider && !isTransitioning) {
                            isClosing = false
                            isTransitioning = true
                            isPlaying = true
                            navPanel.visible = false
                            frameController.startWithDefaultFiles()
                            updateImages()
                            initialTimer.start()
                        } else {
                            console.log("Play blocked: isTransitioning =", isTransitioning)
                        }
                    }
                }

                Button {
                    id: toggleNavButton
                    width: root.height * 0.025
                    height: root.height * 0.0333
                    x: leftImage.x
                    y: leftImage.y
                    z: 2
                    background: Rectangle {
                        color: "transparent"
                        radius: 4
                    }
                    hoverEnabled: true
                    scale: hovered ? 1.1 : 1.0
                    Behavior on scale { NumberAnimation { duration: 200 } }

                    onClicked: {
                        navPanel.visible = !navPanel.visible
                    }
                    contentItem: Canvas {
                        id: hamburgerCanvas
                        width: root.height * 0.0148
                        height: root.height * 0.0148
                        anchors.centerIn: parent
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.fillStyle = "#0069d9"
                            ctx.lineWidth = height * 0.125
                            var lineHeight = height * 0.125
                            var spacing = height * 0.1875
                            for (var i = 0; i < 3; i++) {
                                ctx.fillRect(2, i * spacing + lineHeight / 2 + 2, width - 4, lineHeight)
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: navPanel
        width: root.width * 0.11
        height: root.height * 0.30
        y: rowLayout.y + playbackRect.y
        color: "#80b3b3b3"
        anchors.left: parent.left
        visible: true
        z: 1
        border.color: "#3399ff"
        border.width: 2

        ColumnLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: root.width * 0.0052
            anchors.topMargin: root.height * 0.042
            spacing: root.height * 0.0093

            Button {
                id: playDefaultButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.0052
                implicitHeight: root.height * 0.025
                hoverEnabled: true
                enabled: !isCameraActive
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (frameProvider && !isTransitioning) {
                        isClosing = false
                        isTransitioning = true
                        isPlaying = true
                        navPanel.visible = false
                        frameController.startWithDefaultFiles()
                        updateImages()
                        initialTimer.start()
                    } else {
                        console.log("Play blocked: isTransitioning =", isTransitioning)
                    }
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Play Default"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: playUserButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.0052
                implicitHeight: root.height * 0.025
                hoverEnabled: true
                enabled: !isCameraActive
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (userFilesModel.count === 0) {
                        emptyFolderPopup.open()
                        return
                    }
                    if (isTransitioning) {
                        console.log("Play user files blocked: isTransitioning =", isTransitioning)
                        return
                    }
                    isClosing = false
                    isTransitioning = true
                    isPlaying = true
                    navPanel.visible = false
                    frameController.playUserFiles()
                    updateImages()
                    initialTimer.start()
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Play User Files"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: addButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.02
                implicitHeight: root.height * 0.025
                hoverEnabled: true
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (!isTransitioning) {
                        openAddFileExplorer()
                    } else {
                        console.log("Add files blocked: isTransitioning =", isTransitioning)
                    }
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Add Files"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: removeButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.02
                implicitHeight: root.height * 0.025
                hoverEnabled: true
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (!isTransitioning) {
                        openRemoveFileExplorer()
                    } else {
                        console.log("Remove files blocked: isTransitioning =", isTransitioning)
                    }
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Remove Files"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: launchCameraButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.0052
                implicitHeight: root.height * 0.025
                hoverEnabled: true
                enabled: !isCameraActive
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (!isTransitioning) {
                        cameraManager.launchCamera()
                    } else {
                        console.log("Launch camera blocked: isTransitioning =", isTransitioning)
                    }
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Launch Camera"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: exitCameraButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.0052
                implicitHeight: root.height * 0.025
                visible: isCameraActive
                hoverEnabled: true
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    cameraManager.stopCamera()
                    isCameraActive = false
                    cameraPreview.source = ""
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Exit Camera"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: closeButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.0052
                implicitHeight: root.height * 0.025
                visible: isPlaying
                hoverEnabled: true
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    isClosing = true
                    initialTimer.stop()
                    regularTimer.stop()
                    leftImage.source = ""
                    rightImage.source = ""
                    frameController.clear()
                    closeDelayTimer.start()
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Close Playback"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }

            Button {
                id: helpButton
                Layout.preferredWidth: root.width * 0.075
                Layout.preferredHeight: root.height * 0.025
                Layout.leftMargin: root.width * 0.0052
                implicitHeight: root.height * 0.025
                hoverEnabled: true
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: helpPopup.open()

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Help"
                    font.pixelSize: root.height * 0.017
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.005
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }
        }
    }

    Button {
        id: exitButton
        width: root.width * 0.04
        height: root.height * 0.03
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: root.width * 0.01
        z: 2
        hoverEnabled: true
        scale: hovered ? 1.1 : 1.0
        Behavior on scale { NumberAnimation { duration: 200 } }

        background: Rectangle {
            color: parent.hovered ? "#0277bd" : "#0069d9"
            radius: 4
            Behavior on color { ColorAnimation { duration: 200 } }
        }

        contentItem: Text {
            text: "Exit"
            font.pixelSize: root.height * 0.013
            color: "#ffffff"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: {
            frameController.clear()
            Qt.quit()
        }
    }

    MouseArea {
        id: outsideClickArea
        anchors.fill: parent
        z: -1
        enabled: navPanel.visible
        propagateComposedEvents: true
        onClicked: {
            var clickPoint = mapToItem(navPanel, mouse.x, mouse.y)
            if (clickPoint.x < 0 || clickPoint.x > navPanel.width ||
                clickPoint.y < 0 || clickPoint.y > navPanel.height) {
                navPanel.visible = false
            }
            mouse.accepted = false
        }
    }

    Timer {
        id: closeDelayTimer
        interval: 200
        running: false
        repeat: false
        onTriggered: {
            isPlaying = false
            isClosing = false
            isTransitioning = false
        }
    }

    Timer {
        id: initialTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered: {
            updateImages()
            regularTimer.start()
        }
    }

    Timer {
        id: regularTimer
        interval: 5000
        running: false
        repeat: true
        onTriggered: {
        frameController.nextFrame()
            updateImages()
        }
    }

    function openAddFileExplorer() {
        if (!fileDialogHelper) {
            return
        }

        var selectedFiles = fileDialogHelper.selectMultipleFiles(
                                "Select PNG Files",
                                ["*.png", "*.PNG"],
                                "/home/root")
        handleAddedFiles(selectedFiles)
    }

    function openRemoveFileExplorer() {
        if (!fileDialogHelper) {
            return
        }

        var selectedFiles = fileDialogHelper.selectMultipleFiles(
                                "Select Files to Remove",
                                ["*.png", "*.PNG"],
                                userFilesDir)
        handleRemovedFiles(selectedFiles)
    }

    function handleAddedFiles(filePaths) {
        var validFiles = []
        var validFilePaths = []
        var invalidFiles = []

        for (var i = 0; i < filePaths.length; i++) {
            var localPath = filePaths[i].toString()
            var info = videoProber.probeFile(localPath)

            if (info.width === 1280 && info.height === 720 && info.depth === 24) {
                validFiles.push(localPath)
                validFilePaths.push(localPath)
            } else {
                var fileName = localPath.split("/").pop()
                invalidFiles.push(fileName)
            }
        }

        if (invalidFiles.length > 0) {
            invalidFilesText = "Unsupported file(s):\n" + invalidFiles.join(", ")
            resolutionPopup.open()
        }

        if (validFiles.length > 0) {
            var destDir = userFilesDir.replace("file://", "")
            if (fileManager.copyFiles(validFilePaths, destDir)) {
                var copiedUrls = []
                for (var j = 0; j < validFiles.length; j++) {
                    var fileNameOut = validFiles[j].split("/").pop()
                    copiedUrls.push(userFilesDir + "/" + fileNameOut)
                }
                frameController.addUserFiles(copiedUrls)
            }
        } else {
            console.log("No valid files to copy")
        }
    }

    function handleRemovedFiles(filePaths) {
        if (filePaths.length > 0) {
            var fileUrls = []
            for (var i = 0; i < filePaths.length; i++) {
                var path = filePaths[i].toString()
                fileUrls.push("file://" + path)
            }

            if (fileManager.deleteFiles(filePaths)) {
                frameController.removeUserFiles(fileUrls)
            } else {
                invalidFilesText = "Failed to delete some files"
                resolutionPopup.open()
            }
        } else {
            console.log("No files selected for removal")
        }
    }

    Connections {
        target: frameController
        function onImageChanged() {
            if (!isClosing) {
                updateImages()
            }
        }
    }

    Connections {
        target: cameraManager
        function onCaptureResult(success, message) {
            isCapturing = false
            if (success) {
                var prefix = "Image captured to "
                if (message.startsWith(prefix)) {
                    lastCapturedImagePath = message.substring(prefix.length)
                }
            }
            capturePopup.open()
        }
    }

    Popup {
        id: resolutionPopup
        width: root.width * 0.15
        height: root.height * 0.075
        modal: false
        focus: true
        x: playButton.x + playButton.width + 20
        y: playButton.y - height / 2
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "#f8d7da"
            border.color: "#f5c2c7"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: invalidFilesText
                color: "#721c24"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - 20
            }
        }

        Timer {
            id: autoCloseTimer
            interval: 3000
            running: false
            repeat: false
            onTriggered: resolutionPopup.close()
        }

        onOpened: autoCloseTimer.start()
    }

    Popup {
        id: emptyFolderPopup
        width: root.width * 0.17
        height: root.height * 0.080
        modal: false
        focus: true
        x: playButton.x + playButton.width + 20
        y: playButton.y - height / 2
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "#f8d7da"
            border.color: "#f5c2c7"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "The user-slideshow-images folder is empty.\nPlease select user files to play."
                color: "#721c24"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - 20
                font.pixelSize: root.height * 0.018
            }
        }

        Timer {
            id: emptyFolderAutoCloseTimer
            interval: 3000
            running: false
            repeat: false
            onTriggered: emptyFolderPopup.close()
        }

        onOpened: emptyFolderAutoCloseTimer.start()
    }

    Popup {
        id: noCameraPopup
        width: root.width * 0.15
        height: root.height * 0.075
        modal: false
        focus: true
        x: playButton.x + playButton.width + 20
        y: playButton.y - height / 2
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "#f8d7da"
            border.color: "#f5c2c7"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "No camera detected.\nPlease connect a camera."
                color: "#721c24"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - 20
                font.pixelSize: root.height * 0.018
            }
        }

        Timer {
            id: noCameraAutoCloseTimer
            interval: 3000
            running: false
            repeat: false
            onTriggered: noCameraPopup.close()
        }

        onOpened: noCameraAutoCloseTimer.start()
    }

    Popup {
        id: capturePopup
        width: root.width * 0.15
        height: root.height * 0.1
        modal: false
        focus: true
        x: playButton.x + playButton.width + 20
        y: playButton.y - height / 2
        closePolicy: Popup.CloseOnEscape

        Rectangle {
            anchors.fill: parent
            color: "#f8d7da"
            border.color: "#f5c2c7"
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    id: capturePopupText
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.bottomMargin: root.height * 0.03
                    color: "#721c24"
                    text: "Image captured."
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: root.height * 0.016
                    maximumLineCount: 3
                    elide: Text.ElideRight
                }

                Button {
                    id: captureOkButton
                    Layout.preferredWidth: parent.width * 0.4
                    Layout.preferredHeight: parent.height * 0.3
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    text: "OK"
                    background: Rectangle {
                        color: parent.hovered ? "#0069d9" : "#0277bd"
                        radius: 4
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: root.height * 0.013
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    hoverEnabled: true
                    scale: hovered ? 1.1 : 1.0
                    Behavior on scale { NumberAnimation { duration: 200 } }
                    onClicked: {
                        if (!isTransitioning && lastCapturedImagePath !== "") {
                            cameraManager.stopCamera()
                            isCameraActive = false
                            cameraPreview.source = ""
                            isClosing = false
                            isTransitioning = true
                            isPlaying = true
                            navPanel.visible = false
                            frameController.playSingleFile(lastCapturedImagePath)
                            updateImages()
                            initialTimer.start()
                            capturePopup.close()
                        } else {
                            console.log("Cannot start slideshow: isTransitioning =", isTransitioning, "or no captured image")
                            capturePopup.close()
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: helpPopup
        width: root.width * 0.25
        height: root.width * 0.15
        modal: false
        focus: true
        x: (root.width - width) / 2
        y: (root.height - height) / 2
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "#ffffff"
            border.color: "#000000"
            radius: 8

            Text {
                id: helpText
                anchors.centerIn: parent
                text: "Images with resolution 1280x720 and bit depth 24 are only supported."
                color: "#000000"
                font.pixelSize: root.height * 0.015
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - 40
            }

            Button {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 20
                text: "Close"
                width: helpPopup.width * 0.2
                height: helpPopup.height * 0.1
                background: Rectangle {
                    color: "#0069d9"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: helpPopup.close()
            }
        }
    }
}

