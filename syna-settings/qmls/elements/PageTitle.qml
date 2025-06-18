import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

TextView {
    id: root
    font.pixelSize: Resource.h2
    font.family: Resource.vollkornRegular.name
    Layout.alignment: Qt.AlignLeft
    text: ""
}