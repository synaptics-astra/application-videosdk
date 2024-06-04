import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

ItemDelegate {
    id: listItem
    property string title: ""
    highlighted: ListView.isCurrentItem
    height: 50
    hoverEnabled: true
    focus: true

    background: Rectangle {
        color: "transparent"
        anchors.fill: parent
        radius: 8
    }

    TextView {
        anchors.centerIn: parent
        opacity: 0.87
        text: title
        font.pixelSize: listItem.highlighted ?  Resource.h3 : Resource.h4
        isBold: listItem.highlighted
        isGray: listItem.highlighted
    }
}
