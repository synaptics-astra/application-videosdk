import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import QtQuick.Window 2.1

import "../custom_views"

Rectangle {
    id: ss
    color: Resource.background

    Item {
        id: spacer_0
        height: Screen.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: ss.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isGray: true
            text: qsTr("Super Resolution")
        }
    }

    GridLayout {
        width: ss.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 5
            image: "qrc:/res/images/camera_icon.png"
            title:  qsTr("Super Resolution")
        }
    }
}

