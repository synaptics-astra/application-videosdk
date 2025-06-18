import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"

Rectangle {
    id:fr
    color: Resource.background

    Item {
        id: spacer_0
        height: Resource.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: fr.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isGray: true
            text: qsTr("Face Recognition")
        }
    }

    GridLayout {
        width: fr.width * 0.9
        anchors.top: header.bottom
        anchors.centerIn: parent

        GridItem {
            type: 6
            image: "qrc:/res/images/camera_icon.png"
            title:  qsTr("Face Recognition")
        }
    }
}

