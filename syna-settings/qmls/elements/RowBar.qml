import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

Rectangle {
    id: blueBar
    width: parent.width
    height: 60
    color: Resource.highlightColor
    radius: 12
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 5

    RowLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 0
    }
}
