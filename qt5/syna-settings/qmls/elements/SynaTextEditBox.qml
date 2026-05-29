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
    property real editBoxWidth: parent.width - 100
    property real editBoxHeight: 40
    signal editingFinished()
    TextView {
        width: 50
        font.pixelSize: Resource.paragraph
        font.family: Resource.vollkornRegular.name
        Layout.alignment: Qt.AlignHCenter
        text: root.label
    }
    TextField {
        // width: parent.width - 100
        Layout.preferredWidth: root.editBoxWidth
        Layout.preferredHeight: root.editBoxHeight
        text: root.value
        placeholderText: root.placeHolderText
        font.pixelSize: Resource.paragraph + 4
        onEditingFinished: {
            root.value = text
            root.editingFinished()
        }
    }
}
