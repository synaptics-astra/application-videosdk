import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

ApplicationWindow {
    visible: true
    visibility: "FullScreen"
    title: qsTr("SL1620 :: Syna Capability Demo")
    color: "#ffffff"
    width: screen.width
    height: screen.height

    function enableButton(buttonID) {
        if(buttonID) {
            if(applicationArea.fileURL1 != "") {
                buttonID.background.color = "#7393b3"
                buttonID.enabled = true
            }
            else {
                buttonID.background.color = "#444444"
                buttonID.enabled = false
            }
        }
    }

    ToolBar {
        id: overlayHeader
        z: 1
        width: parent.width
        height: parent.height * 0.1
        Rectangle {
            width: parent.width
            height: parent.height
            color: "black"
            Image {
                source: "qrc:/images/synaptics_logo.jpg"
                width: parent.width * 0.075
                height: parent.height * 0.75
                Rectangle {
                    width: parent.width
                    height: parent.height
                    color: "transparent"
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins : parent.width * 0.025
                }
            }

            Label {
                id: label
                anchors.centerIn: parent
                text: "Syna Capability Demo"
                font.pixelSize: 40
                font.bold: true
                color: "white"
            }
        }
    }

    Rectangle {
        id: applicationArea
        x: 0
        width: parent.width
        height: parent.height - overlayHeader.height
        anchors.top: overlayHeader.bottom
        color: "transparent"

        property var isSaveConfigEnable : false
        property var capabilityDemoFiles : []
        property string fileURL1 : ""

        property string url1 : "glvideomixer name=comp background=1
                    sink_0::xpos=0 sink_0::ypos=0 sink_0::width=640 sink_0::height=360
                    sink_1::xpos=640 sink_1::ypos=0 sink_1::width=640 sink_1::height=360
                    ! video/x-raw, format=BGRA ! waylandsink
                    multifilesrc loop=true location=" + fileURL1 + " ! h264parse ! avdec_h264 thread-type=slice ! queue ! comp.sink_0
                    CAMSRC ! comp.sink_1"

        property var synaCapabilityDemoConfigPath : "/home/root/demos/configs/syna_capability_demo_sl1620_config.txt"

        Component.onCompleted: {
            applicationArea.capabilityDemoFiles = fileControl.readFromFile(synaCapabilityDemoConfigPath);
            if(applicationArea.capabilityDemoFiles.length >= 1) {
                applicationArea.fileURL1 = applicationArea.capabilityDemoFiles[0].toString()
                button_fileSelector1.text = applicationArea.fileURL1
                button_fileSelector1.background.color = "#7393b3"
            }
            enableButton(play)
        }

        Item {
            id: spacer_start
            height: parent.height * 0.01
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: fileSelectorArea
            width: parent.width * 0.45
            implicitHeight: parent.height
            anchors.top: spacer_start.bottom
            anchors.left: applicationArea.left
            color: "#ffffff"

            Item {
                id: spacer_00
                height: parent.height * 0.30
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Rectangle {
                id: fileSelector1
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.16
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: spacer_00.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog1
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL1 = fileDialog1.fileUrl.toString().replace("file://", "")
                        button_fileSelector1.text = applicationArea.fileURL1
                        button_fileSelector1.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL1 = ""
                        button_fileSelector1.text = "Select Window Item 1"
                        button_fileSelector1.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_01
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector1
                    text: "Select Window Item 1"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_01.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog1.open()
                    }
                }
            }

        }

        Rectangle {
            id: controlArea
            width: parent.width * 0.45
            implicitHeight: parent.height
            anchors.top: spacer_start.bottom
            anchors.right: parent.right
            color: "#ffffff"

            Item {
                id: spacer_x1
                height: parent.height * 0.20
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Button {
                id : play
                text: "PLAY"
                font.pixelSize: 30
                width: parent.width * 0.9
                height: parent.height * 0.1
                highlighted : true
                anchors.top: spacer_x1.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: false
                background: Rectangle {
                    width: parent.width
                    implicitHeight: parent.height
                    opacity: enabled ? 1 : 0.3
                    color: play.down ? "#444444" : "#444444"
                }
                onClicked: {
                        playbackControl.doPlayback(applicationArea.url1, "")
                    }
            }

            Item {
                id: spacer_x2
                height: parent.height * 0.05
                width: parent.width
                anchors.top: play.bottom
            }

            Button {
                id : store_files
                text: "SAVE SETTINGS"
                font.pixelSize: 30
                width: parent.width * 0.9
                height: parent.height * 0.1
                highlighted : true
                anchors.top: spacer_x2.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: applicationArea.isSaveConfigEnable ? true : false
                background: Rectangle {
                    width: parent.width
                    implicitHeight: parent.height
                    opacity: enabled ? 1 : 0.3
                    color: store_files.down ? "#7393b3" : "#7393b3"
                }
                onClicked: {
                    applicationArea.capabilityDemoFiles = []
                    applicationArea.capabilityDemoFiles.length = 0
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL1)
                    fileControl.writeToFile(applicationArea.synaCapabilityDemoConfigPath, applicationArea.capabilityDemoFiles);
                    applicationArea.isSaveConfigEnable = false
                }
            }

            Item {
                id: spacer_x3
                height: parent.height * 0.05
                width: parent.width
                anchors.top: store_files.bottom
            }

            Button {
                id : exit
                text: "Exit"
                font.pixelSize: 30
                width: parent.width * 0.9
                height: parent.height * 0.1
                highlighted : true
                anchors.top: spacer_x3.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                background: Rectangle {
                    width: parent.width
                    implicitHeight: parent.height
                    opacity: enabled ? 1 : 0.3
                    color: exit.down ? "#444444" : "#444444"
                }
                onClicked: {
                    Qt.quit()
                }
            }
        }
    }
}
