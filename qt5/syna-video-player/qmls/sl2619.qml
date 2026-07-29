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
    title: machineHostName === "sl2619-coralboard"
           ? qsTr("Syna Video Player - SL2619-coralboard")
           : qsTr("Syna Video Player - SL2619")
    color: "#ffffff"

    property var quadAllowedFileBases: [
        "astra_building_blocks",
        "astra_edge_ai",
        "astra_multimedia",
        "astra_unboxing"
    ]
    property var cmdsModel: null
    property var rtspUrlsModel: null
    property string url1: ""
    property string url2: ""
    property string url3: ""
    property string url4: ""
    property int settingsIdRtspUrls: 2 // Corrected to 1 based on SettingsManager.cpp enum
    property string cameraDefaultToken: "CAMSRC ! placeholdersink"
    property string camera480pPipeline: "CAMSRC ! videoscale ! video/x-raw,width=640,height=480 ! placeholdersink"
    property string rtsp480pCaps: "video/x-h264, width=640, height=480"

    function shouldShowSourceForCurrentView(sourceValue) {
        if (!sourceValue || singleView.checked || sourceValue.indexOf("filesrc location=") === -1) {
            return true
        }

        if (quadView.checked) {
            var match = sourceValue.match(/\/([^\/]+)\.mp4/)
            var baseName = match ? match[1] : ""
            return quadAllowedFileBases.indexOf(baseName) !== -1
        }

        return true
    }

    function mapFileSourceTo480p(source) {
        return source.replace(/(filesrc\s+location=[^\s]+?)(\.mp4)(?=\s|$)/, function(fullMatch, pathPrefix, extension) {
            return /_480p$/.test(pathPrefix) ? pathPrefix + extension : pathPrefix + "_480p" + extension
        })
    }

    function mapSourceForView(source, isQuadView) {
        if (!source) {
            return source
        }

        var mappedSource = source

        if (mappedSource.indexOf("filesrc location=") !== -1) {
            if (isQuadView) {
                mappedSource = mapFileSourceTo480p(mappedSource)
            }
        }

        if (mappedSource.indexOf("CAMSRC") !== -1) {
            mappedSource = mappedSource.replace(cameraDefaultToken, camera480pPipeline)
        }

        if (mappedSource.indexOf("RTSPURL") !== -1 || mappedSource.indexOf("rtspsrc ") !== -1) {
            mappedSource = mappedSource.replace("video/x-h264, width=1920, height=1080", rtsp480pCaps)
        }

        return mappedSource
    }

    Component.onCompleted: {
        var cmdComponent
        var mode = playbackControl.getMode()
        if (mode === "ffmpeg") {
            cmdComponent = Qt.createComponent("file:///home/root/demos/qmls/ffmpegcommands.qml")
        }
        cmdsModel = cmdComponent.createObject()

        try {
            var rtspData = settingsManager.getData(settingsIdRtspUrls)
            rtspUrlsModel = rtspData && rtspData["rtspUrls"] ? rtspData["rtspUrls"] : ["", "", "", ""]
        } catch (error) {
            console.error("Error in settingsManager.getData:", error.message)
            rtspUrlsModel = ["", "", "", ""]
        }
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

    Popup {
        id: dialog
        modal: true
        focus: true
        anchors.centerIn: player.contentItem

        contentItem: Item {
            ColumnLayout {
                spacing: 10
                Layout.alignment: Qt.AlignCenter

                Rectangle {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 20
                    color: "#f0f0f0"
                    border.color: "#ccc"
                    Text {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignCenter
                        text: "Edit URLs"
                        font.bold: true
                    }
                }

                Repeater {
                    id: repeater
                    model: 4
                    Layout.topMargin: 20
                    delegate: RowLayout {
                        spacing: 10
                        Layout.fillWidth: true

                        Text {
                            text: "RTSP URL " + (index + 1)
                            Layout.fillWidth: true
                        }

                        TextField {
                            text: rtspUrlsModel && rtspUrlsModel.length > index ? rtspUrlsModel[index] : ""
                            placeholderText: "Enter URL"
                            background: Rectangle {
                                color: "white"
                                border.color: "#3498db"
                                border.width: 1
                                radius: 4
                            }
                            Layout.fillWidth: true
                        }
                    }
                }

                RowLayout {
                    spacing: 10
                    width: parent.width
                    Button {
                        text: "OK"
                        highlighted: true
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: update.down ? "#000000" : "#a9a9a9"
                            radius: 20
                        }
                        onClicked: dialog.close()
                    }
                }
            }
        }

        onClosed: {
            var urls = []
            for (var i = 0; i < 4; i++) {
                var row = repeater.itemAt(i)
                if (row) {
                    for (var j = 0; j < row.children.length; j++) {
                        var child = row.children[j]
                        if (child.hasOwnProperty("placeholderText")) {
                            urls.push(child.text)
                        }
                    }
                } else {
                    console.error("Row not found")
                }
            }
            var rtspData = { "rtspUrls": urls }
            try {
                var success = settingsManager.setData(settingsIdRtspUrls, rtspData)
                if (!success) {
                    console.error("settingsManager.setData failed")
                }
            } catch (error) {
                console.error("Error in settingsManager.setData:", error.message)
            }
        }
    }

    ColumnLayout {
        id: mainLayout
        spacing: 10
        width: player.width

        Rectangle {
            color: "black"
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            height: player.height / 12

            Image {
                source: "qrc:/images/astra_white.png"
                sourceSize: Qt.size(100, 100)
            }

            Label {
                id: label
                text: "Astra Video Playback Demo"
                font.pixelSize: player.height / 25
                font.bold: true
                color: "white"
                anchors.centerIn: parent
            }
        }

        RowLayout {
            id: secondLayer
            spacing: 10
            Layout.leftMargin: player.height / 25
            Layout.rightMargin: player.height / 25
            Layout.topMargin: player.height / 35
            Layout.bottomMargin: 0
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft

            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true
                Repeater {
                    model: 4
                    delegate: Label {
                        enabled: !singleView.checked || index === 0
                        text: "VIDEO " + (index + 1)
                        font.pixelSize: 30
                        Layout.fillWidth: true
                        Layout.preferredHeight: player.height / 15
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        background: Rectangle {
                            opacity: enabled ? 1 : 0.3
                            color: "#6495ed"
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true

                Repeater {
                    model: 4
                    delegate: ComboBox {
                        id: comboBox
                        enabled: !singleView.checked || index === 0
                        textRole: "text"
                        valueRole: "value"
                        model: player.cmdsModel
                        font.pixelSize: 30
                        width: parent.width * 0.8
                        Layout.fillWidth: true
                        Layout.preferredHeight: player.height / 15

                        function resetToFirstValidIndex() {
                            if (!model || model.count === undefined) {
                                return
                            }
                            for (var i = 0; i < model.count; i++) {
                                if (player.shouldShowSourceForCurrentView(model.get(i).value)) {
                                    currentIndex = i
                                    return
                                }
                            }
                        }

                        Connections {
                            target: quadView
                            function onCheckedChanged() {
                                comboBox.resetToFirstValidIndex()
                            }
                        }

                        Connections {
                            target: singleView
                            function onCheckedChanged() {
                                comboBox.resetToFirstValidIndex()
                            }
                        }

                        delegate: ItemDelegate {
                            font.pixelSize: 30
                            width: comboBox.width
                            visible: player.shouldShowSourceForCurrentView(model.value)
                            height: visible ? implicitHeight : 0
                            contentItem: Text {
                                text: model.text
                                font: comboBox.font
                            }
                        }

                        onCurrentIndexChanged: {
                            player["url" + (index + 1)] = valueAt(currentIndex)
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.topMargin: player.height / 25
            Layout.fillWidth: true
            Button {
                id: update
                text: "Update RTSP Urls"
                font.pixelSize: player.height / 35
                highlighted: true
                Layout.leftMargin: player.height / 25
                background: Rectangle {
                    color: update.down ? "#000000" : "#a9a9a9"
                    radius: 20
                }

                onClicked: {
                    dialog.open()
                }
            }
        }

        RowLayout {
            Layout.topMargin: player.height / 25
            Layout.preferredWidth: parent.width / 4
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            RadioButton {
                id: singleView
                text: "SINGLE VIEW"
                font.pixelSize: player.height / 35
                Layout.alignment: Qt.AlignLeft
                checked: true
            }

            RadioButton {
                id: quadView
                text: "QUAD VIEW"
                font.pixelSize: player.height / 35
                Layout.alignment: Qt.AlignRight
            }
        }

        ColumnLayout {
            Layout.topMargin: player.height / 25
            Layout.leftMargin: player.height / 25
            Layout.rightMargin: player.height / 25
            Layout.preferredWidth: parent.width / 2
            Layout.alignment: Qt.AlignHCenter

            Button {
                id: play
                text: "PLAY"
                font.pixelSize: player.height / 35
                highlighted: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                Layout.preferredHeight: player.height / 18
                background: Rectangle {
                    opacity: enabled ? 1 : 0.3
                    color: play.down ? "#000000" : "#a9a9a9"
                }
                onClicked: {
                    if (singleView.checked) {
                        playbackControl.doPlayback(1, mapSourceForView(player.url1, false), "", "", "")
                    } else if (quadView.checked) {
                        playbackControl.doPlayback(2,
                            mapSourceForView(player.url1, true),
                            mapSourceForView(player.url2, true),
                            mapSourceForView(player.url3, true),
                            mapSourceForView(player.url4, true))
                    }
                }
            }

            Button {
                id: exit
                text: "Exit"
                font.pixelSize: player.height / 35
                highlighted: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                Layout.preferredHeight: player.height / 18
                background: Rectangle {
                    opacity: enabled ? 1 : 0.3
                    color: exit.down ? "#000000" : "#a9a9a9"
                }
                onClicked: Qt.quit()
            }
        }
    }
}

