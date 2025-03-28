import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0
import QtQuick.Dialogs 1.3

import "../custom_views"
import "../panels"
import "../elements"

Rectangle {
    id: root
    color: Resource.background
    anchors.fill: parent

    // Display Settings Title
    PageTitle {
        text: qsTr("Display Config")
        Layout.alignment: Qt.AlignHCenter
    }

    ColumnLayout {
        id: section
        width: root.width
        anchors.top: parent.top
        anchors.centerIn: parent
        Layout.alignment: Qt.AlignHCenter
        spacing: 20

        // ListModel for Dropdown (Dynamically Updated)
        ListModel {
            id: dropdownListModel
        }

        // Dropdown for Panel Selection
        SynaDropDownMenu {
            id: panelSelector
            label: "Select Panel"
            width: root.width * 0.5
            height: root.height * 0.05
            model: dropdownListModel
            Layout.alignment: Qt.AlignHCenter
            onModelChanged: {
                if (dropdownListModel.count > 0) {
                    value = dropdownListModel.get(0).value
                } else {
                    value = ""
                }
            }
        }

        // Buttons Row (Save, Reset, Print)
        RowLayout {
            id: buttonRow
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            SynaButton {
                id: savePanelButton
                text: qsTr("Save Panel")
                width: root.width * 0.2
                height: root.height * 0.05
                onClicked: {
                    if (!panelSelector.value || panelSelector.value === "") {
                        console.log("Error: No panel selected")
                        return
                    }
                    displaySettings.savePanel(panelSelector.value)
                    var data = { "selectedPanel": panelSelector.value }
                    settingsControl.setData(0, data)
                    confirmDialog.open()
                }
            }

            SynaButton {
                id: resetButton
                text: qsTr("Reset to Default")
                width: root.width * 0.2
                height: root.height * 0.05
                onClicked: {
                    displaySettings.resetToDefault()
                    var SETTINGS_ID_DISPLAY_SETTINGS = 0;
                    settingsControl.deleteData(SETTINGS_ID_DISPLAY_SETTINGS)
                    confirmDialog.open()
                }
            }

            SynaButton {
                id: printInfobutton
                text: qsTr("Print Panel Info")
                width: root.width * 0.2
                height: root.height * 0.05
                onClicked: displaySettings.printPanelInfo()
            }
        }
    }

    // Status Message
    SectionLabel {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        id: statusMessage
        text: ""
        font.pixelSize: root.width * 0.02
        color: "blue"
        wrapMode: Text.Wrap
    }

    // Backend Connections
    Connections {
        target: displaySettings

        function onAvailablePanelsUpdated(panels) {
            dropdownListModel.clear() // Clear the model before populating

            if (panels.length === 0) {
                dropdownListModel.append({ "text": "No panels found", "value": "" })
            } else {
                for (var i = 0; i < panels.length; i++) {
                    dropdownListModel.append({
                        "text": panels[i].text,
                        "value": panels[i].value
                    })
                }
            }
                            if (dropdownListModel.count > 0) {
                    panelSelector.value = dropdownListModel.get(0).value;
                }
        }

        function onStatusUpdated(message) {
            console.log("Status: " + message)
            statusMessage.text = message
        }
    }

    Popup {
        id: confirmDialog
        modal: true
        focus: true
        width: 400
        height: 200
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            radius: 10
            border.color: "black"
            border.width: 1
        }

        ColumnLayout {
            spacing: 10
            anchors.centerIn: parent

            Text {
                text: qsTr("You need to reboot for the changes to take effect.\nWould you like to reboot now?")
                wrapMode: Text.Wrap
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: 20
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 20
                Layout.fillWidth: true

                SynaButton {
                    text: qsTr("Reboot")
                    Layout.preferredWidth: 120
                    height: 40
                    onClicked: {
                        displaySettings.executeCommand("sync && reboot")
                        confirmDialog.close()
                    }
                }

                SynaButton {
                    text: qsTr("Cancel")
                    Layout.preferredWidth: 120
                    height: 40
                    onClicked: confirmDialog.close()
                }
            }
        }
    }

    // Populate dropdown on component load
    Component.onCompleted: {
        displaySettings.fetchAvailablePanels()
    }
}
