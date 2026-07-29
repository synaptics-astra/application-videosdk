import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Resource 1.0
import "../custom_views"
import "../elements"
import QtQuick.Window 2.15

Rectangle {
    id: root
    color: Resource.background ? Resource.background : "white"
    anchors.fill: parent

    PageTitle {
        id: pageTitle
        text: qsTr("RTSP Camera")
        Layout.alignment: Qt.AlignHCenter
    }

    ColumnLayout {
        id: section
        width: root.width * 0.8
        anchors.centerIn: parent
        spacing: 20

        Text {
            id: listHeading
            text: qsTr("RTSP URLs")
            font.pixelSize: root.height * 0.030
            Layout.alignment: Qt.AlignHCenter
        }

        ListModel {
            id: rtspUrlsModel
            Component.onCompleted: {
                var SETTINGS_ID_RTSP_URLS = 2;
                var defaultUrls = ["", "", "", ""];

                if (!settingsControl.checkData(SETTINGS_ID_RTSP_URLS)) {
                    settingsControl.setData(SETTINGS_ID_RTSP_URLS, { rtspUrls: defaultUrls });
                }

                var savedData = settingsControl.getData(SETTINGS_ID_RTSP_URLS);
                var urls = defaultUrls;
                if (savedData && savedData["rtspUrls"]) {
                    urls = savedData["rtspUrls"].slice(0, 4);
                    while (urls.length < 4) {
                        urls.push("");
                    }
                }

                clear();
                for (var i = 0; i < urls.length; i++) {
                    append({ url: urls[i] });
                }
            }
        }

        Repeater {
            model: rtspUrlsModel
            SynaTextEditBox {
                label: qsTr("RTSP URL" + (index + 1) + ":")
                placeHolderText: qsTr("Enter RTSP URL")
                value: model.url
                editBoxWidth: root.width * 0.35
                editBoxHeight: root.height * 0.05
                Layout.alignment: Qt.AlignHCenter
                onEditingFinished: {
                    rtspUrlsModel.setProperty(index, "url", value);
                    var urls = [];
                    for (var i = 0; i < rtspUrlsModel.count; i++) {
                        urls.push(rtspUrlsModel.get(i).url);
                    }
                    var SETTINGS_ID_RTSP_URLS = 2;
                    var data = { "rtspUrls": urls };
                    if (!settingsControl.setData(SETTINGS_ID_RTSP_URLS, data)) {
                         console.log("Failed to save RTSP URLs");
                    }
                }
            }
        }

        RowLayout {
            id: buttonRow
            Layout.topMargin: root.height * 0.03
            Layout.alignment: Qt.AlignHCenter
            spacing: root.width * 0.01

            SynaButton {
                id: saveButton
                text: qsTr("Save")
                font.pixelSize: root.height * 0.02
                Layout.preferredWidth: root.width * 0.10
                height: root.height * 0.055
                onClicked: {
                    var urls = [];
                    for (var i = 0; i < rtspUrlsModel.count; i++) {
                        urls.push(rtspUrlsModel.get(i).url);
                    }
                    var SETTINGS_ID_RTSP_URLS = 2;
                    var data = { "rtspUrls": urls };
                    if (!settingsControl.setData(SETTINGS_ID_RTSP_URLS, data)) {
                        console.log("Failed to save RTSP URLs");
                    }
                }
            }

            SynaButton {
                id: validateButton
                text: qsTr("Validate")
                font.pixelSize: root.height * 0.02
                Layout.preferredWidth: root.width * 0.10
                height: root.height * 0.055
                onClicked: {
                    var urls = [];
                    for (var i = 0; i < rtspUrlsModel.count; i++) {
                        var url = rtspUrlsModel.get(i).url;
                        if (url) urls.push(url);
                    }
                    if (cppRtspConfig && typeof cppRtspConfig.validateAllRtspUrls === "function") {
                        var results = cppRtspConfig.validateAllRtspUrls(urls);
                        rtspInfoDialog.rtspValidationResults = results;
                        rtspInfoDialog.currentIndex = 0;
                        rtspInfoDialog.currentRstpResult = results[0] || {};
                        rtspInfoDialog.visible = true;
                    } else {
                        console.log("Error: rtspConfig.validateAllRtspUrls is not a function");
                        rtspInfoDialog.rtspValidationResults = [];
                        rtspInfoDialog.currentIndex = 0;
                        rtspInfoDialog.currentRstpResult = {};
                        rtspInfoDialog.visible = true;
                    }
                }
            }

            Dialog {
                id: rtspInfoDialog
                title: qsTr("RTSP Stream Information")
                standardButtons: Dialog.Ok
                visible: false
                modal: true
                width: root.width * 0.3
                height: root.height * 0.6
                anchors.centerIn: parent

                property var rtspValidationResults: []
                property int currentIndex: 0
                property var currentRstpResult: ({})

                ColumnLayout {
                    spacing: root.height * 0.01
                    anchors.margins: root.width * 0.02

                    Text {
                        text: qsTr("Address: ") + (rtspInfoDialog.currentRstpResult.url || "Unknown")
                        font.bold: true
                        font.pixelSize: root.height * 0.015
                    }

                    Text {
                        text: qsTr("Status: ") + (rtspInfoDialog.currentRstpResult.status || "Unknown")
                        font.pixelSize: root.height * 0.011
                    }

                    Text {
                        text: qsTr("Message: ") + (rtspInfoDialog.currentRstpResult.message || "No additional info")
                        font.pixelSize: root.height * 0.011
                    }

                    Repeater {
                        model: Object.keys(rtspInfoDialog.currentRstpResult).filter(function(key) { return key.startsWith("stream_"); })

                        Column {
                            spacing: root.height * 0.005
                            Text {
                                text: qsTr("Stream: ") + modelData
                                font.bold: true
                                font.pixelSize: root.height * 0.011
                            }

                            Text {
                                text: qsTr("Type: ") + (rtspInfoDialog.currentRstpResult[modelData].type || "Unknown")
                                font.pixelSize: root.height * 0.01
                            }

                            Text {
                                text: qsTr("Format: ") + (rtspInfoDialog.currentRstpResult[modelData].format || "Unknown")
                                font.pixelSize: root.height * 0.01
                            }

                            Text {
                                visible: rtspInfoDialog.currentRstpResult[modelData].type === "video"
                                text: qsTr("Resolution: ") + (rtspInfoDialog.currentRstpResult[modelData].resolution || "Unknown")
                                font.pixelSize: root.height * 0.01
                            }

                            Text {
                                visible: rtspInfoDialog.currentRstpResult[modelData].type === "video"
                                text: qsTr("Framerate: ") + (rtspInfoDialog.currentRstpResult[modelData].framerate || "Unknown")
                                font.pixelSize: root.height * 0.01
                            }
                        }
                    }

                    RowLayout {
                        spacing: root.width * 0.01
                        SynaButton {
                            text: qsTr("Previous")
                            width: root.width * 0.06
                            height: root.height * 0.024
                            font.pixelSize: root.height * 0.015
                            enabled: rtspInfoDialog.currentIndex > 0
                            onClicked: {
                                rtspInfoDialog.currentIndex--;
                                rtspInfoDialog.currentRstpResult = rtspInfoDialog.rtspValidationResults[rtspInfoDialog.currentIndex];
                            }
                        }
                        SynaButton {
                            text: qsTr("Next")
                            width: root.width * 0.06
                            height: root.height * 0.024
                            font.pixelSize: root.height * 0.015
                            enabled: rtspInfoDialog.currentIndex < rtspInfoDialog.rtspValidationResults.length - 1
                            onClicked: {
                                rtspInfoDialog.currentIndex++;
                                rtspInfoDialog.currentRstpResult = rtspInfoDialog.rtspValidationResults[rtspInfoDialog.currentIndex];
                            }
                        }
                    }
                }
            }
        }
    }
}