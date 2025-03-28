import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

Rectangle {
    id: root
    color: Resource.background

    ColumnLayout {
        width: root.width
        spacing: 20

        PageTitle {
            text: "Home"
        }

        TextView {
            font.pixelSize: Resource.paragraph
            font.family: Resource.cabal.name
            text: "This is synaptics Settings application. It can help to configure settings of diferent modules which can be navigated from left side menu..!"
        }
    }
}
