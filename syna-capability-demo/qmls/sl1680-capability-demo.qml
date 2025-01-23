import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

ApplicationWindow {
    visible: true
    visibility: "FullScreen"
    title: qsTr("SL1680 :: Syna Capability Demo")
    color: "#ffffff"
    width: screen.width
    height: screen.height

    function enableButton(buttonID) {
        if(buttonID) {
            if((applicationArea.fileURL1 != "") && (applicationArea.fileURL2) &&
                (applicationArea.fileURL3) && (applicationArea.fileURL4) &&
                (applicationArea.fileURL5) && (applicationArea.fileURL6) &&
                (applicationArea.fileURL7) && (applicationArea.fileURL8)) {
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
        property string fileURL2 : ""
        property string fileURL3 : ""
        property string fileURL4 : ""
        property string fileURL5 : ""
        property string fileURL6 : ""
        property string fileURL7 : ""
        property string fileURL8 : ""

        property string url1 : "glvideomixer name=comp background=1
                    sink_0::xpos=0 sink_0::ypos=0 sink_0::width=382 sink_0::height=210
                    sink_1::xpos=0 sink_1::ypos=212 sink_1::width=382 sink_1::height=210
                    sink_2::xpos=0 sink_2::ypos=424 sink_2::width=382 sink_2::height=210
                    sink_3::xpos=0 sink_3::ypos=636 sink_3::width=382 sink_3::height=210
                    sink_4::xpos=0 sink_4::ypos=848 sink_4::width=382 sink_4::height=210
                    sink_5::xpos=384 sink_5::ypos=0 sink_5::width=382 sink_5::height=210
                    sink_6::xpos=768 sink_6::ypos=0 sink_6::width=382 sink_6::height=210
                    sink_7::xpos=1152 sink_7::ypos=0 sink_7::width=382 sink_7::height=210
                    sink_8::xpos=1536 sink_8::ypos=0 sink_8::width=382 sink_8::height=210
                    ! video/x-raw, format=NV12 ! waylandsink
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL1 + " ! h264parse ! v4l2h264dec ! comp.sink_0
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL2 + " ! h264parse ! v4l2h264dec ! comp.sink_1
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL3 + " ! h264parse ! v4l2h264dec ! comp.sink_2
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL4 + " ! h264parse ! v4l2h264dec ! comp.sink_3
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL5 + " ! h264parse ! v4l2h264dec ! comp.sink_4
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL6 + " ! h264parse ! v4l2h264dec ! comp.sink_5
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL7 + " ! h264parse ! v4l2h264dec ! comp.sink_6
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL8 + " ! h264parse ! v4l2h264dec ! comp.sink_7
                    CAMSRC ! comp.sink_8"
        property string url2 : "HRXSRC ! waylandsink"

        property var synaCapabilityDemoConfigPath : "/home/root/demos/configs/syna_capability_demo_sl1680_config.txt"

        Component.onCompleted: {
            applicationArea.capabilityDemoFiles = fileControl.readFromFile(synaCapabilityDemoConfigPath);
            if(applicationArea.capabilityDemoFiles.length >= 8) {
                applicationArea.fileURL1 = applicationArea.capabilityDemoFiles[0].toString()
                button_fileSelector1.text = applicationArea.fileURL1
                button_fileSelector1.background.color = "#7393b3"
                applicationArea.fileURL2 = applicationArea.capabilityDemoFiles[1].toString()
                button_fileSelector2.text = applicationArea.fileURL2
                button_fileSelector2.background.color = "#7393b3"
                applicationArea.fileURL3 = applicationArea.capabilityDemoFiles[2].toString()
                button_fileSelector3.text = applicationArea.fileURL3
                button_fileSelector3.background.color = "#7393b3"
                applicationArea.fileURL4 = applicationArea.capabilityDemoFiles[3].toString()
                button_fileSelector4.text = applicationArea.fileURL4
                button_fileSelector4.background.color = "#7393b3"
                applicationArea.fileURL5 = applicationArea.capabilityDemoFiles[4].toString()
                button_fileSelector5.text = applicationArea.fileURL5
                button_fileSelector5.background.color = "#7393b3"
                applicationArea.fileURL6 = applicationArea.capabilityDemoFiles[5].toString()
                button_fileSelector6.text = applicationArea.fileURL6
                button_fileSelector6.background.color = "#7393b3"
                applicationArea.fileURL7 = applicationArea.capabilityDemoFiles[6].toString()
                button_fileSelector7.text = applicationArea.fileURL7
                button_fileSelector7.background.color = "#7393b3"
                applicationArea.fileURL8 = applicationArea.capabilityDemoFiles[7].toString()
                button_fileSelector8.text = applicationArea.fileURL8
                button_fileSelector8.background.color = "#7393b3"
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

            Rectangle {
                id: fileSelector1
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelectorArea.top
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

            Rectangle {
                id: fileSelector2
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector1.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog2
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL2 = fileDialog2.fileUrl.toString().replace("file://", "")
                        button_fileSelector2.text = applicationArea.fileURL2
                        button_fileSelector2.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL2 = ""
                        button_fileSelector2.text = "Select Window Item 2"
                        button_fileSelector2.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_02
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector2
                    text: "Select Window Item 2"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_02.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog2.open()
                    }
                }
            }

            Rectangle {
                id: fileSelector3
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector2.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog3
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL3 = fileDialog3.fileUrl.toString().replace("file://", "")
                        button_fileSelector3.text = applicationArea.fileURL3
                        button_fileSelector3.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL3 = ""
                        button_fileSelector3.text = "Select Window Item 3"
                        button_fileSelector3.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_03
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector3
                    text: "Select Window Item 3"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_03.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog3.open()
                    }
                }
            }

            Rectangle {
                id: fileSelector4
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector3.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog4
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL4 = fileDialog4.fileUrl.toString().replace("file://", "")
                        button_fileSelector4.text = applicationArea.fileURL4
                        button_fileSelector4.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL4 = ""
                        button_fileSelector4.text = "Select Window Item 4"
                        button_fileSelector4.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_04
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector4
                    text: "Select Window Item 4"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_04.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog4.open()
                    }
                }
            }

            Rectangle {
                id: fileSelector5
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector4.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog5
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL5 = fileDialog5.fileUrl.toString().replace("file://", "")
                        button_fileSelector5.text = applicationArea.fileURL5
                        button_fileSelector5.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL5 = ""
                        button_fileSelector5.text = "Select Window Item 5"
                        button_fileSelector5.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_05
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector5
                    text: "Select Window Item 5"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_05.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog5.open()
                    }
                }
            }

            Rectangle {
                id: fileSelector6
                width: parent.width *0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector5.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog6
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL6 = fileDialog6.fileUrl.toString().replace("file://", "")
                        button_fileSelector6.text = applicationArea.fileURL6
                        button_fileSelector6.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL6 = ""
                        button_fileSelector6.text = "Select Window Item 6"
                        button_fileSelector6.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_06
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector6
                    text: "Select Window Item 6"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_06.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog6.open()
                    }
                }
            }

            Rectangle {
                id: fileSelector7
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector6.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog7
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL7 = fileDialog7.fileUrl.toString().replace("file://", "")
                        button_fileSelector7.text = applicationArea.fileURL7
                        button_fileSelector7.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL7 = ""
                        button_fileSelector7.text = "Select Window Item 7"
                        button_fileSelector7.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_07
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector7
                    text: "Select Window Item 7"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_07.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog7.open()
                    }
                }
            }

            Rectangle {
                id: fileSelector8
                width: parent.width *0.9
                implicitHeight: parent.height * 0.12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector7.bottom
                // anchors.margins: parent.width *0.9
                color: "#ffffff"

                //File Dialog
                FileDialog {
                    id: fileDialog8
                    title: "Select a File"
                    folder: shortcuts.home
                    selectMultiple: false
                    nameFilters: ["H264 Files (*.h264)"]
                    onAccepted: {
                        applicationArea.fileURL8 = fileDialog8.fileUrl.toString().replace("file://", "")
                        button_fileSelector8.text = applicationArea.fileURL8
                        button_fileSelector8.background.color = "#7393b3"
                        applicationArea.isSaveConfigEnable = true;
                        enableButton(play)
                    }
                    onRejected: {
                        applicationArea.fileURL8 = ""
                        button_fileSelector8.text = "Select Window Item 8"
                        button_fileSelector8.background.color = "#444444"
                        enableButton(play)
                    }
                }

                Item {
                    id: spacer_08
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector8
                    text: "Select Window Item 8"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_08.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    onClicked: {
                        fileDialog8.open()
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
                        playbackControl.doPlayback(applicationArea.url1, applicationArea.url2)
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
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL2)
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL3)
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL4)
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL5)
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL6)
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL7)
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL8)
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
