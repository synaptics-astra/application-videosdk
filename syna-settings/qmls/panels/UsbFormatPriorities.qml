import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Resource 1.0
import "../custom_views"
import "../elements"
import QtQuick.Window 2.15

Rectangle {
    id: root
    color: Resource.background
    anchors.fill: parent

    PageTitle {
        id: pageTitle
        text: qsTr("USB Camera")
        Layout.alignment: Qt.AlignHCenter
    }

    ColumnLayout {
        id: section
        width:root.width * 0.8
        anchors.centerIn: parent
        spacing: 20

        Text {
            id: listHeading
            text: qsTr("Priority List")
            font.pixelSize: root.height * 0.030
            Layout.alignment: Qt.AlignHCenter
        }

        ListModel {
            id: formatListModel
            ListElement { text: "YUY2"; value: "YUY2" }
            ListElement { text: "NV12"; value: "NV12" }
            ListElement { text: "H264"; value: "H264" }
            ListElement { text: "MJPEG"; value: "MJPEG"}
        }

        SynaOrderList {
            id: formatList
            listWidth: root.width * 0.8
            listHeight: root.height * 0.6
            listModel: formatListModel
            buttonWidth: root.width * 0.065
            buttonHeight: root.height * 0.040
            itemFontSize: root.height * 0.02
            itemSpacing: root.width * 0.035
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: section.spacing
        }

        RowLayout {
            id: buttonRow
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: root.height * 0.03
            spacing: root.width * 0.01

            SynaButton {
                id: resetButton
                text: qsTr("Reset")
                font.pixelSize: root.height * 0.02
                Layout.preferredWidth: root.width * 0.10
                height: root.height * 0.055
                onClicked: {
                    var SETTINGS_ID_USB_FORMAT_PRIORITIES = 1;
                    settingsControl.deleteData(SETTINGS_ID_USB_FORMAT_PRIORITIES)
                    formatListModel.clear()
                    formatListModel.append({ text: "YUY2", value: "YUY2"})
                    formatListModel.append({ text: "NV12", value: "NV12"})
                    formatListModel.append({ text: "H264",  value: "H264"})
                    formatListModel.append({ text: "MJPEG", value: "MJPEG"})
                }
            }

            SynaButton {
                id: saveButton
                text: qsTr("Save")
                font.pixelSize: root.height * 0.02
                Layout.preferredWidth: root.width * 0.10
                height: root.height * 0.055
                onClicked: {
                    var priorities = []
                    for (var i = 0; i < formatListModel.count; i++) {
                        priorities.push(formatListModel.get(i).value)
                    }
                    var data = {"formatPriorities": priorities }
                    var SETTINGS_ID_USB_FORMAT_PRIORITIES = 1;
                    if (!settingsControl.setData(SETTINGS_ID_USB_FORMAT_PRIORITIES, data)) {
                        console.log("Failed to save USB Format Priorities.")
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        var SETTINGS_ID_USB_FORMAT_PRIORITIES = 1;
        var defaultPriorities = ["YUY2", "NV12", "H264", "MJPEG"];
        var formats = defaultPriorities;

        if (!settingsControl.checkData(SETTINGS_ID_USB_FORMAT_PRIORITIES)) {
            settingsControl.setData(SETTINGS_ID_USB_FORMAT_PRIORITIES, { formatPriorities: defaultPriorities });
        }

        var savedData = settingsControl.getData(SETTINGS_ID_USB_FORMAT_PRIORITIES);
        if (savedData && savedData["formatPriorities"] && Array.isArray(savedData["formatPriorities"])) {
            formats = savedData["formatPriorities"].slice(0, 4);
            while (formats.length < 4) {
                formats.push("");
            }
        }

        formatListModel.clear();
        for (var i = 0; i < formats.length; i++) {
            formatListModel.append({ text: formats[i], value: formats[i] });
        }
    }
}