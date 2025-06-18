import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

TextView {
    id: root
    var title: ""
    font.pixelSize: Resource.h2
    font.family: Resource.nasa21.name
    Layout.alignment: Qt.AlignLeft
    text: root.title
}