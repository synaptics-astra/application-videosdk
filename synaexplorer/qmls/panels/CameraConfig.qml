import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Resource 1.0
import "../custom_views"
import QtQuick.Window 2.15

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
            Layout.leftMargin: root.width * 0.05
            font.pixelSize: Resource.h2
            isBold: true
            isHighlight: true
            text: qsTr("Settings")
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: Screen.height * 0.01
                anchors.margins: root.width * 0.02

                Column {
                    spacing: Screen.height * 0.015
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: root.width * 0.6

                    Text {
                        text: "USB Camera Format Priorities"
                        font.bold: true
                        font.pointSize: root.height / 60
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: root.width * 0.01
                        rowSpacing: Screen.height * 0.005
                        width: parent.width

                        Column {
                            Layout.preferredWidth: root.width * 0.1
                            Layout.fillHeight: true
                            spacing: Screen.height * 0.005

                            Repeater {
                                model: settingsManager ? settingsManager.selectedFormats.length : 0
                                Rectangle {
                                    width: parent.width
                                    height: Screen.height * 0.03
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
                            spacing: Screen.height * 0.005

                            Repeater {
                                model: settingsManager ? settingsManager.selectedFormats : []
                                Row {
                                    width: parent.width
                                    height: Screen.height * 0.03
                                    spacing: root.width * 0.005

                                    Rectangle {
                                        width: parent.width * 0.7
                                        height: Screen.height * 0.03
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
                                            width: root.width * 0.03
                                            height: Screen.height * 0.015
                                            enabled: index > 0
                                            onClicked: settingsManager.reorderPriority(index, index - 1)
                                        }
                                        Button {
                                            text: "▼"
                                            width: root.width * 0.03
                                            height: Screen.height * 0.015 // Reduced by 20%
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
                    spacing: root.width * 0.01
                    Layout.alignment: Qt.AlignCenter
                    Button {
                        text: "Reset Priorities"
                        width: root.width * 0.1
                        height: Screen.height * 0.024 // Reduced by 20% from 0.03
                        onClicked: settingsManager.resetPriorities()
                    }
                    Button {
                        text: "Save Priorities"
                        width: root.width * 0.1
                        height: Screen.height * 0.024 // Reduced by 20%
                        onClicked: settingsManager.saveConfig("/home/root/demos/configs/synaexplorer_config.txt")
                    }
                }
            }
        }

        Item {
            Layout.preferredHeight: Screen.height * 0.01
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: "#B0BEC5"
            Layout.leftMargin: root.width * 0.02
            Layout.rightMargin: root.width * 0.02
        }
        Item {
            Layout.preferredHeight: Screen.height * 0.01
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: Screen.height * 0.01
                anchors.margins: root.width * 0.02

                Column {
                    spacing: Screen.height * 0.015
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: root.width * 0.6

                    Text {
                        text: "RTSP URLs Testing"
                        font.bold: true
                        font.pointSize: root.height / 60
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: root.width * 0.01
                        rowSpacing: Screen.height * 0.005
                        width: parent.width

                        Column {
                            Layout.preferredWidth: root.width * 0.1
                            Layout.fillHeight: true
                            spacing: Screen.height * 0.005

                            Repeater {
                                model: 4
                                Rectangle {
                                    width: parent.width
                                    height: Screen.height * 0.03 // Reduced by 20% from 0.0375
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
                            spacing: Screen.height * 0.005

                            Repeater {
                                model: rtspUrlsModel
                                TextField {
                                    width: parent.width * 0.7
                                    height: Screen.height * 0.03 // Reduced by 20% from 0.0375
                                    text: modelData
                                    placeholderText: "Enter RTSP URL"
                                    background: Rectangle {
                                        color: "#F5F5F5"
                                        border.color: "black"
                                        radius: 4
                                    }
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
                    spacing: root.width * 0.01
                    Layout.alignment: Qt.AlignCenter
                    Button {
                        text: "Validate URLs"
                        width: root.width * 0.1
                        height: Screen.height * 0.024 // Reduced by 20% from 0.03
                        onClicked: {
                            rtspValidationResults = settingsManager.validateAllRtspUrls();
                            currentIndex = 0;
                            currentRtspResult = rtspValidationResults[0] || {};
                            rtspInfoDialog.visible = true;
                        }
                    }
                    Button {
                        text: "Save URLs"
                        width: root.width * 0.1
                        height: Screen.height * 0.024 // Reduced by 20%
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
        width: root.width * 0.3
        height: Screen.height * 0.6

        ColumnLayout {
            spacing: Screen.height * 0.01
            anchors.margins: root.width * 0.02

            Text {
                text: "URL: " + (currentRtspResult.url || "Unknown")
                font.bold: true
                font.pointSize: root.height / 100
            }

            Text {
                text: "Status: " + (currentRtspResult.status || "Unknown")
                font.pointSize: root.height / 60
            }

            Text {
                text: "Message: " + (currentRtspResult.message || "No additional info")
                font.pointSize: root.height / 60
            }

            Repeater {
                model: Object.keys(currentRtspResult).filter(key => key.startsWith("stream_"))

                Column {
                    spacing: Screen.height * 0.005
                    Text {
                        text: "Stream: " + modelData
                        font.bold: true
                        font.pointSize: root.height / 80
                    }
                    Text {
                        text: "Type: " + (currentRtspResult[modelData].type || "Unknown")
                        font.pointSize: root.height / 80
                    }
                    Text {
                        text: "Format: " + (currentRtspResult[modelData].format || "Unknown")
                        font.pointSize: root.height / 80
                    }

                    Component.onCompleted: console.log("Media Type: ", currentRtspResult[modelData].type)
                    Text {
                        visible: currentRtspResult[modelData].type === "video"
                        text: "Resolution: " + (currentRtspResult[modelData].resolution || "Unknown")
                        font.pointSize: root.height / 80
                    }
                    Text {
                        visible: currentRtspResult[modelData].type === "video"
                        text: "Framerate: " + (currentRtspResult[modelData].framerate || "Unknown")
                        font.pointSize: root.height / 80
                    }
                }
            }

            RowLayout {
                spacing: root.width * 0.01
                Button {
                    text: "Previous"
                    width: root.width * 0.060
                    height: Screen.height * 0.024
                    enabled: currentIndex > 0
                    onClicked: {
                        currentIndex--;
                        currentRtspResult = rtspValidationResults[currentIndex];
                    }
                }

                Button {
                    text: "Next"
                    width: root.width * 0.060
                    height: Screen.height * 0.024
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
