import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

ColumnLayout {
    id: root
    property var label: ""
    property var listModel: ""
    property int maxTextWidth: 0  // Store the max text width

    spacing: 10
    anchors.margins: 10

    TextView {
        visible: (label.length != 0)
        font.pixelSize: Resource.paragraph
        font.family: Resource.vollkornRegular.name
        Layout.alignment: Qt.AlignHCenter
        text: root.label
    }

    Repeater {
        id: repeaterList
        model: listModel
        Row {
            spacing: 5

            Button {
                text: "▼"
                width: 40
                height: 18
                enabled: index < (repeaterList.count - 1)
                onClicked: {
                    listModel.move(index, index+1,1)
                }
            }

            Text {
                width: maxTextWidth
                text: model.text
                font.pointSize: 12
            }

            Button {
                text: "▲"
                width: 40
                height: 18
                enabled: index > 0
                onClicked: {
                    listModel.move(index, index-1,1)
                }
            }
        }
    }

    // Hidden text element for width calculation
    Text {
        id: textHelper
        visible: false
        font.pixelSize: 16
    }

    Component.onCompleted: {
        var maxWidth = 0
        for (var i = 0; i < listModel.count; i++) {
            textHelper.text = listModel.get(i).text
            maxWidth = Math.max(maxWidth, textHelper.implicitWidth)
        }
        maxTextWidth = maxWidth
    }

}
