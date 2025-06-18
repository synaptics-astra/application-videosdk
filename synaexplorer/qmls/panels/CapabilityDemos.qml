import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"

Rectangle {
    id:root
    color: Resource.background

    Item {
        id: spacer_0
        height: Resource.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: root.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isHighlight: true
            text: qsTr("Capability Demos")
        }
    }

    GridLayout {
        width: root.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 7
            image: "qrc:/res/images/astra_icon.png"
            title:  qsTr("Best Case")
        }
    }
}
