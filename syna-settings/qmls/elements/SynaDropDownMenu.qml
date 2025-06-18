import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"


RowLayout {
    id: root
    property var label: "Dropdown box label"
    property var value: 0
    property var model: ""

    TextView {
        width: 50
        font.pixelSize: Resource.paragraph
        font.family: Resource.vollkornRegular.name
        Layout.alignment: Qt.AlignHCenter
        text: root.label
    }

    ComboBox {
        id: comboBox
        textRole: "text"
        valueRole: "value"
        model: root.model
        font.pixelSize: Resource.paragraph
        Layout.preferredHeight: 40
        Layout.preferredWidth: 500
        delegate: ItemDelegate {
            font.pixelSize: Resource.paragraph
            contentItem: Text {
                text: model.text
                font: comboBox.font
            }
        }

        onCurrentIndexChanged: {
            root.value = valueAt(currentIndex);
        }
    }

    onValueChanged: {
        comboBox.currentIndex = root.value
    }

    Component.onCompleted: {
        comboBox.currentIndex = root.value
    }
}