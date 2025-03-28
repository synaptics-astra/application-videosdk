import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

RowLayout {
    id: root
    property var placeHolderText: "Enter value"
    property var value: "Field Value"
    property var label: "Edit box label"
    TextView {
        width: 50
        font.pixelSize: Resource.paragraph
        font.family: Resource.vollkornRegular.name
        Layout.alignment: Qt.AlignHCenter
        text: root.label
    }
    TextField {
        width: parent.width - 100
        Layout.preferredHeight: 40
        text: root.value
        placeholderText: root.placeHolderText
        onEditingFinished: {
            root.value = text
        }
    }
}
