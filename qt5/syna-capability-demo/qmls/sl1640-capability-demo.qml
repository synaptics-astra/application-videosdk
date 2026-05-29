import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import "."

ApplicationWindow {
    id: player
    visible: true
    visibility: "FullScreen"
    title: qsTr("SL1640 :: Syna Capability Demo")
    color: "#ffffff"
    width: screen.width
    height: screen.height

    property int selectedFileIndex: 0
    property string selectedFilePath: ""
    property bool filePathUpdated: false
    property var buttonArray: []

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

    function enableButton(buttonID) {
        if(buttonID) {
            if((applicationArea.fileURL1 != "") && (applicationArea.fileURL2) &&
                (applicationArea.fileURL3) && (applicationArea.fileURL4) &&
                (applicationArea.fileURL5) && (applicationArea.fileURL6) &&
                (applicationArea.fileURL7) && (applicationArea.fileURL8) &&
                (applicationArea.fileURL9)) {
                buttonID.background.color = "#7393b3"
                buttonID.enabled = true
            }
            else {
                buttonID.background.color = "#444444"
                buttonID.enabled = false
            }
        }
    }

    onFilePathUpdatedChanged: {
        if (player.filePathUpdated) {
            var index = player.selectedFileIndex;
            if (index >= 0 && index < player.buttonArray.length) {
                var fileURLKey = "fileURL" + (index + 1);
                var buttonId = "button_fileSelector" + (index + 1);

                applicationArea[fileURLKey] = player.selectedFilePath;

                var button = player.buttonArray[(player.buttonArray.length - 1) - player.selectedFileIndex]
                button.text = player.selectedFilePath;
                button.background.color = "#7393b3";
            }

            applicationArea.isSaveConfigEnable = true;
            enableButton(play);
        }
        player.filePathUpdated = false;
    }


    CustomFileDialog {
        id: customFileDialog
        onUpdateProperty: {
            player.selectedFilePath = value;
            player.filePathUpdated = true
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
                font.pixelSize: player.height / 25
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
        property string fileURL9 : ""

        property string url1 : "glvideomixer name=comp background=1
                    sink_0::xpos=0 sink_0::ypos=0 sink_0::width=640 sink_0::height=360
                    sink_1::xpos=0 sink_1::ypos=360 sink_1::width=640 sink_1::height=360
                    sink_2::xpos=0 sink_2::ypos=720 sink_2::width=640 sink_2::height=360
                    sink_3::xpos=640 sink_3::ypos=0 sink_3::width=640 sink_3::height=360
                    sink_4::xpos=640 sink_4::ypos=360 sink_4::width=640 sink_4::height=360
                    sink_5::xpos=640 sink_5::ypos=720 sink_5::width=640 sink_5::height=360
                    sink_6::xpos=1280 sink_6::ypos=0 sink_6::width=640 sink_6::height=360
                    sink_7::xpos=1280 sink_7::ypos=360 sink_7::width=640 sink_7::height=360
                    sink_8::xpos=1280 sink_8::ypos=720 sink_8::width=640 sink_8::height=360
                    ! video/x-raw, format=NV12 ! placeholdersink
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL1 + " ! h264parse ! v4l2h264dec ! comp.sink_0
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL2 + " ! h264parse ! v4l2h264dec ! comp.sink_1
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL3 + " ! h264parse ! v4l2h264dec ! comp.sink_2
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL4 + " ! h264parse ! v4l2h264dec ! comp.sink_3
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL5 + " ! h264parse ! v4l2h264dec ! comp.sink_4
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL6 + " ! h264parse ! v4l2h264dec ! comp.sink_5
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL7 + " ! h264parse ! v4l2h264dec ! comp.sink_6
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL8 + " ! h264parse ! v4l2h264dec ! comp.sink_7
                    multifilesrc loop=true caps=\"video/x-h264, framerate=25/1\"  location=" + fileURL8 + " ! h264parse ! v4l2h264dec ! comp.sink_8"

        property var synaCapabilityDemoConfigPath : "/home/root/demos/configs/syna_capability_demo_sl1640_config.txt"

        Component.onCompleted: {
            applicationArea.capabilityDemoFiles = fileControl.readFromFile(synaCapabilityDemoConfigPath);
            if(applicationArea.capabilityDemoFiles.length >= 9) {
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
                applicationArea.fileURL9 = applicationArea.capabilityDemoFiles[8].toString()
                button_fileSelector9.text = applicationArea.fileURL9
                button_fileSelector9.background.color = "#7393b3"
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
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelectorArea.top
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector1)
                    }
                    onClicked: {
                        player.selectedFileIndex = 0
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector2
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector1.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector2)
                    }
                    onClicked: {
                        player.selectedFileIndex = 1
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector3
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector2.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector3)
                    }
                    onClicked: {
                        player.selectedFileIndex = 2
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector4
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector3.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector4)
                    }
                    onClicked: {
                        player.selectedFileIndex = 3
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector5
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector4.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector5)
                    }
                    onClicked: {
                        player.selectedFileIndex = 4
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector6
                width: parent.width *0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector5.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector6)
                    }
                    onClicked: {
                        player.selectedFileIndex = 5
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector7
                width: parent.width * 0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector6.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector7)
                    }
                    onClicked: {
                        player.selectedFileIndex = 6
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector8
                width: parent.width *0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector7.bottom
                color: "#ffffff"

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
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector8)
                    }
                    onClicked: {
                        player.selectedFileIndex = 7
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
                    }
                }
            }

            Rectangle {
                id: fileSelector9
                width: parent.width *0.9
                implicitHeight: parent.height * 0.10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: fileSelector8.bottom
                color: "#ffffff"

                Item {
                    id: spacer_09
                    height: parent.height * 0.3
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Button {
                    id : button_fileSelector9
                    text: "Select Window Item 9"
                    font.pixelSize: 30
                    width: parent.width
                    height: parent.height * 0.7
                    highlighted : true
                    anchors.top: spacer_09.bottom
                    background: Rectangle {
                        width: parent.width
                        implicitHeight: parent.height
                        opacity: enabled ? 1 : 0.3
                        color: "#444444"
                    }
                    Component.onCompleted: {
                        player.buttonArray.push(button_fileSelector9)
                    }
                    onClicked: {
                        player.selectedFileIndex = 8
                        customFileDialog.fileDialogVisible = true
                        customFileDialog.updateFileList()
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
                property bool isSingleClick: true
                property int clickThreshold: 1000
                background: Rectangle {
                    width: parent.width
                    implicitHeight: parent.height
                    opacity: enabled ? 1 : 0.3
                    color: play.down ? "#444444" : "#444444"
                }
                Timer {
                    id: waitTimer
                    interval: play.clickThreshold
                    running: false
                    repeat: false
                    onTriggered: {
                        play.isSingleClick = true
                    }
                }
                onClicked: {
                    if(play.isSingleClick) {
                        play.isSingleClick = false
                        playbackControl.doPlayback(applicationArea.url1, "")
                        waitTimer.start();
                    }
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
                    applicationArea.capabilityDemoFiles.push(applicationArea.fileURL9)
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
