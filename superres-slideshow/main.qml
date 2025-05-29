import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.3
import Qt.labs.folderlistmodel 2.15

ApplicationWindow {
    id: root
    width: 3840
    height: 2160
    visible: true
    visibility: "FullScreen"
    title: qsTr("SR Slideshow")
    color: "#0069d9" // Fallback background, matches header

    property var leftImageSource
    property var rightImageSource
    property string invalidFilesText: ""
    property bool isClosing: false
    property bool isTransitioning: false
    property bool isPlaying: false
    property string userFilesDir: "file:///home/root/demos/user-slideshow-images"

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0" // Matches title bar shade
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
        leftImage.source = "image://frameProvider/normalFrame?t=" + timestamp
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
                }

                Button {
                    id: playButton
                    width: root.height * 0.04 * 1.6 + 10
                    height: root.height * 0.04 * 1.6
                    anchors.centerIn: parent
                    z: 1
                    visible: leftImage.source.toString() === "" && rightImage.source.toString() === ""
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
                            navPanel.visible = false // Hide nav panel on play
                            frameProvider.startWithDefaultFiles()
                            updateImages()
                            initialTimer.start()
                            // mouse.accepted = true
                        } else {
                            console.log("Play blocked: isTransitioning =", isTransitioning)
                            mouse.accepted = true
                        }
                    }
                }

                // Toggle Button (Hamburger)
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
        height: root.height * 0.25
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
                scale: hovered ? 1.1 : 1.0
                Behavior on scale { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (frameProvider && !isTransitioning) {
                        isClosing = false
                        isTransitioning = true
                        isPlaying = true
                        navPanel.visible = false // Hide nav panel on play
                        frameProvider.startWithDefaultFiles()
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
                    font.pixelSize: root.height * 0.018
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.0026
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
                    navPanel.visible = false // Hide nav panel on play
                    frameProvider.playUserFiles()
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
                    font.pixelSize: root.height * 0.018
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.0026
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
                        fileDialog.open()
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
                    font.pixelSize: root.height * 0.018
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.0026
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
                        removeFileDialog.open()
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
                    font.pixelSize: root.height * 0.018
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.0026
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
                    if (frameProvider) {
                        if (frameProvider.clear) {
                            frameProvider.clear()
                        } else if (frameProvider.stop) {
                            frameProvider.stop()
                        }
                    }
                    closeDelayTimer.start()
                }

                background: Rectangle {
                    color: parent.hovered ? "#0069d9" : "transparent"
                    radius: 4
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                contentItem: Text {
                    text: "Close Playback"
                    font.pixelSize: root.height * 0.018
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.0026
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
                    font.pixelSize: root.height * 0.018
                    color: parent.hovered ? "#ffffff" : "#000000"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: root.width * 0.0026
                    Layout.fillWidth: true
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }
        }
    }

    // Exit Button
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
            if (frameProvider) {
                if (frameProvider.clear) {
                    frameProvider.clear()
                } else if (frameProvider.stop) {
                    frameProvider.stop()
                }
            }
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
            updateImages()
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select PNG Files"
        nameFilters: ["PNG files (*.png)"]
        selectMultiple: true
        onAccepted: {
            var validFiles = []
            var validFilePaths = []
            var invalidFiles = []

            for (var i = 0; i < fileUrls.length; i++) {
                var url = fileUrls[i]
                var localPath = url.toString().replace("file://", "")
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
                        var fileName = validFiles[j].split("/").pop()
                        copiedUrls.push(userFilesDir + "/" + fileName)
                    }
                    frameProvider.addUserFiles(copiedUrls)
                }
            } else {
                console.log("No valid files to copy")
            }
        }
    }

    FileDialog {
        id: removeFileDialog
        title: "Select Files to Remove"
        folder: userFilesDir
        nameFilters: ["PNG files (*.png)"]
        selectMultiple: true
        onAccepted: {
            if (fileUrls.length > 0) {
                var filePaths = []
                for (var i = 0; i < fileUrls.length; i++) {
                    var filePath = fileUrls[i].toString().replace("file://", "")
                    filePaths.push(filePath)
                }
                if (fileManager.deleteFiles(filePaths)) {
                    frameProvider.removeUserFiles(fileUrls)
                } else {
                    invalidFilesText = "Failed to delete some files"
                    resolutionPopup.open()
                }
            } else {
                console.log("No files selected for removal")
            }
        }
    }

    Connections {
        target: frameProvider
        function onImageChanged() {
            if (!isClosing) {
                updateImages()
            }
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
                text: "The user-slideshow-images folder is empty.\nPlease select user files to play."
                color: "#721c24"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - 20
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