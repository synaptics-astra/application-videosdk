import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

Rectangle {
    id: root
    width: 80
    height: 36
    radius: 18
    color: checked ? "#4CAF50" : "#B0BEC5"
    border.color: Resource.whiteColor
    border.width: 2
    Layout.alignment: Qt.AlignVCenter

    Rectangle {
        id: toggleCircle
        width: 32
        height: 32
        radius: 16
        color: "white"
        anchors.verticalCenter: parent.verticalCenter
        x: checked ? parent.width - width - 2 : 2

        Behavior on x {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutCubic
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            checked = !checked
            toggleStateChanged(checked)
        }
    }

    signal toggleStateChanged(bool checked)
    property bool checked: false
}
