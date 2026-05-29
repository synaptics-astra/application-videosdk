import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Window 2.1

import com.synaptics.AppSink 1.0

Window {
    width: 1280
    height: 720
    visible: true
    visibility: "FullScreen"

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
    }


    function data_feed()
    {
        var cload = appSink.getcpuload();
        var gload = appSink.getgpuload();
        var nload = appSink.getnpuload();

        cpuload.text = cload+"%";
        cpuLoadBar.value = cload / 100

        gpuload.text = gload+"%";
        gpuLoadBar.value = gload / 100

        npuload.text = nload+"%";
        npuLoadBar.value = nload / 100
    }

    Timer {
        interval: 3000; running: true; repeat: true
        onTriggered: data_feed()
    }

    AppSink {
        id: appSink
        onSendString: {
            var lines = string.split(":");
            if (lines.length >= 4) {
                label1.text = lines[0];
                conf1.text = lines[1];
                label2.text = lines[2];
                conf2.text = lines[3];
            } else {
                console.error("Invalid data format");
            }
        }
    }

    Pane {
        id: root
        width: parent.width
        height: parent.height
        background: Rectangle { color: "#f7edec" }

        property bool fullscreen : false

        TapHandler {
            onDoubleTapped: {
                if (root.fullscreen) {
                    showNormal()
                    root.fullscreen = false
                } else {
                    showFullScreen()
                    root.fullscreen = true
                }
            }
        }

        Component.onCompleted: {
            appSink.createPipeline();
        }

        ColumnLayout {
            anchors.fill: parent
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                color: "#007dc3"
                Layout.alignment: Qt.AlignTop

                RowLayout {
                    anchors.fill: parent
                    Image {
                        source: "qrc:/res/images/astra_icon.png"
                        sourceSize: Qt.size(parent.height, parent.height)
                        Layout.leftMargin: 20
                        Layout.alignment: Qt.AlignLeft
                    }

                    Text {
                        text: "Audio Classification"
                        font.pixelSize: 40
                        color: "white"
                        Layout.rightMargin: 20
                        Layout.alignment: Qt.AlignRight
                        wrapMode: Text.WordWrap
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Text {
                    id: labelHead
                    font {
                        bold: true
                        weight: Font.Bold
                        pixelSize: 30
                    }
                    text: "Model - Yamnet.tflite"
                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop

                }


                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 120
                    Layout.rightMargin: 120

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            text: "CLASS"
                            font { pixelSize: 50; bold: true; underline: true }
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: "Confidence"
                            font { pixelSize: 50; bold: true; underline: true }
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            id: label1
                            text: ""
                            font { pixelSize: 40; bold: true }
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            id: conf1
                            text: ""
                            font { pixelSize: 40; bold: true }
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            id: label2
                            text: ""
                            font { pixelSize: 40; bold: true }
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            id: conf2
                            text: ""
                            font { pixelSize: 40; bold: true }
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }


                Text {
                    id: label
                    font {
                        pixelSize: 40
                        bold: true
                    }
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                id: splitter
                Layout.fillWidth: true
                Layout.preferredHeight: 2
                color: "#000000"
            }

            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true
                Layout.leftMargin: 120
                Layout.rightMargin: 120
                Layout.preferredHeight: 250

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("CPU Load:")
                        font {
                            bold: true
                            weight: Font.Bold
                            pixelSize: 40
                        }
                    }

                    Text {
                        id: cpuload
                        text: qsTr("")
                        font.pixelSize: 40
                        Layout.alignment: Qt.AlignRight
                    }

                    ProgressBar {
                        id: cpuLoadBar
                        Layout.fillWidth: true
                        value: 30 / 100
                    }

                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("GPU Load:")
                        font {
                            bold: true
                            weight: Font.Bold
                            pixelSize: 40
                        }
                    }

                    Text {
                        id: gpuload
                        text: qsTr("")
                        font.pixelSize: 40
                        Layout.alignment: Qt.AlignRight
                    }

                    ProgressBar {
                        id: gpuLoadBar
                        Layout.fillWidth: true
                        value: 30 / 100
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: qsTr("NPU Load:")
                        font {
                            bold: true
                            weight: Font.Bold
                            pixelSize: 40
                        }
                    }

                    Text {
                        id: npuload
                        text: qsTr("")
                        font.pixelSize: 40
                        Layout.alignment: Qt.AlignRight
                    }

                    ProgressBar {
                        id: npuLoadBar
                        Layout.fillWidth: true
                        value: 30 / 100
                    }
                }
            }

            ToolBar {
                id: toolBar
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBottom

                ToolButton {
                    text: qsTr("&Exit")
                    onClicked: Qt.quit()
                    anchors.right: parent.right
                    font.pixelSize: 20
                }
            }
        }

    }
}
