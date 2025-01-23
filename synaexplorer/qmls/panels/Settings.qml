import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Resource 1.0
import "../custom_views"

Rectangle {
    id: root
    color: Resource.background
    Layout.fillWidth: true
    Layout.fillHeight: true
    radius: 4

    property var rtspValidationResults: []
    property var currentRtspResult: ({})
    property int currentIndex: 0
    property var rtspUrlsModel: null

    Component.onCompleted: {
        var rtspUrls = settingsManager.getRtspUrls()
        if (rtspUrls === null || rtspUrls.length === 0) {
            rtspUrlsModel = ["", "", "", ""]
        } else {
            rtspUrlsModel = rtspUrls
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isHighlight: true
            text: qsTr("Settings")
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignCenter
            Layout.leftMargin: 10
            Layout.rightMargin: 10

            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                anchors.margins: 10

                Text {
                    text: "USB Camera Format Priorities"
                    font.bold: true
                    font.pointSize: 14
                    Layout.alignment: Qt.AlignCenter
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignCenter

                    GridLayout {
                        anchors.centerIn: parent
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 5
                        width: parent.width * 0.6

                        Column {
                            Layout.preferredWidth: 120
                            Layout.fillHeight: true
                            spacing: 5

                            Repeater {
                                model: settingsManager.selectedFormats.length
                                Rectangle {
                                    width: parent.width
                                    height: 40
                                    color: "#E0E0E0"
                                    border.color: "black"
                                    radius: 4

                                    Text {
                                        text: "Priority " + (index + 1)
                                        anchors.centerIn: parent
                                        font.bold: true
                                        font.pointSize: 12
                                    }
                                }
                            }
                        }

                        Column {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 5

                            Repeater {
                                model: settingsManager.selectedFormats
                                Row {
                                    width: parent.width
                                    height: 40
                                    spacing: 5

                                    Rectangle {
                                        width: parent.width - 100
                                        height: 40
                                        color: "#F5F5F5"
                                        border.color: "black"
                                        radius: 4

                                        Text {
                                            text: modelData
                                            anchors.centerIn: parent
                                            font.pointSize: 12
                                        }
                                    }

                                    Column {
                                        spacing: 2
                                        Button {
                                            text: "▲"
                                            width: 40
                                            height: 18
                                            enabled: index > 0
                                            onClicked: settingsManager.reorderPriority(index, index - 1)
                                        }
                                        Button {
                                            text: "▼"
                                            width: 40
                                            height: 18
                                            enabled: index < settingsManager.selectedFormats.length - 1
                                            onClicked: settingsManager.reorderPriority(index, index + 1)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Row {
                    spacing: 10
                    Layout.alignment: Qt.AlignCenter
                    Button {
                        text: "Reset Priorities"
                        onClicked: settingsManager.resetPriorities()
                    }
                    Button {
                        text: "Save Priorities"
                        onClicked: settingsManager.saveConfig("/home/root/demos/configs/synaexplorer_config.txt")
                    }
                }
            }
        }

        Item {
            Layout.preferredHeight: 10
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: "#B0BEC5"
            Layout.leftMargin: 10
            Layout.rightMargin: 10
        }
        Item {
            Layout.preferredHeight: 10
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignCenter
            Layout.leftMargin: 10
            Layout.rightMargin: 10

            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                anchors.margins: 10

                Text {
                    text: "RTSP URLs Testing"
                    font.bold: true
                    font.pointSize: 14
                    Layout.alignment: Qt.AlignCenter
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignCenter

                    GridLayout {
                        anchors.centerIn: parent
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 5
                        width: parent.width * 0.6

                        Column {
                            Layout.preferredWidth: 120
                            Layout.fillHeight: true
                            spacing: 5

                            Repeater {
                                model: 4
                                Rectangle {
                                    width: parent.width
                                    height: 40
                                    color: "#E0E0E0"
                                    border.color: "black"
                                    radius: 4

                                    Text {
                                        text: "RTSP URL " + (index + 1) + ":"
                                        anchors.centerIn: parent
                                        font.bold: true
                                        font.pointSize: 10
                                    }
                                }
                            }
                        }

                        Column {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 5

                            Repeater {
                                model: rtspUrlsModel
                                TextField {
                                    width: parent.width - 100
                                    Layout.preferredHeight: 40
                                    text: modelData
                                    placeholderText: "Enter RTSP URL"
                                    onEditingFinished: {
                                        var urls = settingsManager.getRtspUrls()
                                        if (index < urls.length)
                                            urls[index] = text;
                                        else 
                                            urls.push(text);

                                        settingsManager.setRtspUrls(urls);
                                    }
                                }
                            }
                        }
                    }
                }

                Row {
                    spacing: 10
                    Layout.alignment: Qt.AlignCenter
                    Button {
                        text: "Validate URLs"
                        onClicked: {
                            rtspValidationResults = settingsManager.validateAllRtspUrls();
                            currentIndex = 0;
                            currentRtspResult = rtspValidationResults[0] || {};
                            rtspInfoDialog.visible = true;
                        }
                    }
                    Button {
                        text: "Save URLs"
                        onClicked: settingsManager.saveConfig("/home/root/demos/configs/synaexplorer_config.txt")
                    }
                }
            }
        }
    }
    Dialog {
        id: rtspInfoDialog
        title: "RTSP Stream Information"
        standardButtons: Dialog.Ok
        visible: false
        modal: true
        width: 500
        height: 600

        ColumnLayout {
            spacing: 10
            anchors.margins: 10

            Text {
                text: "URL: " + (currentRtspResult.url || "Unknown")
                font.bold: true
            }

            Text {
                text: "Status: " + (currentRtspResult.status || "Unknown")
            }

            Text {
                text: "Message: " + (currentRtspResult.message || "No additional info")
            }

            Repeater {
                model: Object.keys(currentRtspResult).filter(key => key.startsWith("stream_"))

                Column {
                    spacing: 5
                    Text {
                        text: "Stream: " + modelData
                        font.bold: true
                    }
                    Text {
                        text: "Type: " + (currentRtspResult[modelData].type || "Unknown")
                    }
                    Text {
                        text: "Format: " + (currentRtspResult[modelData].format || "Unknown")
                    }

                    Component.onCompleted: console.log("Media Type: ", currentRtspResult[modelData].type)
                    Text {
                        visible: currentRtspResult[modelData].type === "video"
                        text: "Resolution: " + (currentRtspResult[modelData].resolution || "Unknown")
                    }
                    Text {
                        visible: currentRtspResult[modelData].type === "video"
                        text: "Framerate: " + (currentRtspResult[modelData].framerate || "Unknown")
                    }
                }
            }

            RowLayout {
                spacing: 10
                Button {
                    text: "Previous"
                    enabled: currentIndex > 0
                    onClicked: {
                        currentIndex--;
                        currentRtspResult = rtspValidationResults[currentIndex];
                    }
                }

                Button {
                    text: "Next"
                    enabled: currentIndex < rtspValidationResults.length - 1
                    onClicked: {
                        currentIndex++;
                        currentRtspResult = rtspValidationResults[currentIndex];
                    }
                }
            }
        }
    }
}

