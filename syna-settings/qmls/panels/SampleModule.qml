import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"


Rectangle {
    id: root
    color: Resource.background
    anchors.fill: parent

    ColumnLayout {
        id: section
        width: root.width
        anchors.top: parent.top
        Layout.leftMargin: 10
        spacing: 10

        /* Page Title which gets displayed on top */
        PageTitle {
            text: "Sample Module"
        }

        /* Section 1 starts */
        SectionLabel {
            text: "Section header for section #1"
        }
        SynaTextEditBox {
            id: textBox1
            Layout.alignment: Qt.AlignLeft
            placeHolderText: "Enter text"
            value: "Textbox current value1"
            label: "TextBox 1"
        }
        SynaTextEditBox {
            id: textBox2
            Layout.alignment: Qt.AlignLeft
            placeHolderText: "Enter text"
            value: "Textbox current value2"
            label: "TextBox 2"
        }

        /* Section 2 starts */
        SectionLabel {
            text: "Section Label for section #2"
        }
        SynaTextEditBox {
            id: textBoxA
            Layout.alignment: Qt.AlignLeft
            placeHolderText: "Enter text"
            value: "Textbox current valueA"
            label: "TextBox A"
        }
        SynaTextEditBox {
            id: textBoxB
            Layout.alignment: Qt.AlignLeft
            placeHolderText: "Enter text"
            value: "Textbox current valueB"
            label: "TextBox B"
        }

        /* Section 3 starts */
        SectionLabel {
            text: "Section Label for section #3"
        }
        SynaDropDownMenu {
            id: dropDownMenu
            label: "Selected Item"
            model: ListModel {
                id: dropdownListModel
                ListElement { text: "Item 1"; value: 0}
                ListElement { text: "Item 2"; value: 1}
                ListElement { text: "Item 3"; value: 2}
                ListElement { text: "Item 4"; value: 3}
            }
        }

        /* Section 4 starts */
        SectionLabel {
            text: "Section Label for section #4"
        }
        SynaOrderList {
            id: orderList
            label: "Order the list based on priority in decending order"
            listModel: ListModel {
                ListElement { text: "Item 1"}
                ListElement { text: "Item 2"}
                ListElement { text: "Item 3"}
                ListElement { text: "Item 4"}
                ListElement { text: "Item 5"}
            }
        }
        /* Sections end here */

        /* Buttons row */
        RowLayout {
            id: buttonRow
            Layout.leftMargin: 10
            spacing: 10

            SynaButton {
                id: saveButton
                text: "Save"
                onClicked : {
                    /* Write the values to settings file */
                    saveValuesToConfig();
                }
            }
            SynaButton {
                id: clearButton
                text: "Clear"
                onClicked : {
                    fetchValuesFromConfig();
                }
            }
            SynaButton {
                id: defaultButton
                text: "Default"
                onClicked : {
                    /* Set default values and write the values to settings file */
                    setLocalValues(getDefaultData());
                    saveValuesToConfig();
                }
            }
        }
    }

    Component.onCompleted: {
        fetchValuesFromConfig();
    }

    function setLocalValues(data) {
        textBox1.value = data.textBox1_str;
        textBox2.value = data.textBox2_str;
        textBoxA.value = data.textBoxA_str;
        textBoxB.value = data.textBoxB_str;
        dropDownMenu.value = data.dropDownMenu_int;
        orderList.listModel.clear();
        orderList.listModel.append({ text: data.dropDownMenu_prio0_str});
        orderList.listModel.append({ text: data.dropDownMenu_prio1_str});
        orderList.listModel.append({ text: data.dropDownMenu_prio2_str});
        orderList.listModel.append({ text: data.dropDownMenu_prio3_str});
        orderList.listModel.append({ text: data.dropDownMenu_prio4_str});
    }

    function getDefaultData() {
        return {
            "textBox1_str": "TextBox 1",
            "textBox2_str": "TextBox 2",
            "textBoxA_str": "TextBox A",
            "textBoxB_str": "TextBox B",
            "dropDownMenu_int": 0,
            "dropDownMenu_prio0_str": "Item 1",
            "dropDownMenu_prio1_str": "Item 2",
            "dropDownMenu_prio2_str": "Item 3",
            "dropDownMenu_prio3_str": "Item 4",
            "dropDownMenu_prio4_str": "Item 5",
        }
    }

    function saveValuesToConfig() {
        var data = {
            "textBox1_str": textBox1.value,
            "textBox2_str": textBox2.value,
            "textBoxA_str": textBoxA.value,
            "textBoxB_str": textBoxB.value,
            "dropDownMenu_int": parseInt(dropDownMenu.value),
            "dropDownMenu_prio0_str": orderList.listModel.get(0).text,
            "dropDownMenu_prio1_str": orderList.listModel.get(1).text,
            "dropDownMenu_prio2_str": orderList.listModel.get(2).text,
            "dropDownMenu_prio3_str": orderList.listModel.get(3).text,
            "dropDownMenu_prio4_str": orderList.listModel.get(4).text,
        }
        settingsControl.setData( 0 /*SETTINGS_ID_SAMPLE_MODULE*/, data);
    }

    function fetchValuesFromConfig() {
        /* Set the current values from file */
        var data = settingsControl.getData( 0 /*SETTINGS_ID_SAMPLE_MODULE*/);

        if (Object.keys(data).length === 0) {
            /* No config file found. Save default values */
            setLocalValues(getDefaultData());
            saveValuesToConfig();
        } else {
            setLocalValues(data);
        }
    }
}

