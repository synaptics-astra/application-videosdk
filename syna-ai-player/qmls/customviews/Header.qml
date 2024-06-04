import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

Image {
    width: Resource.headerWidth
    height: Resource.headerHeight
    source: "qrc:/res/images/headerbanner.png"

    RowLayout {
        width: parent.width - 20
        height: parent.height
        x: 10
        Image {
            source: "qrc:/res/images/astraicon.png"
            sourceSize: Qt.size(parent.height, parent.height)
        }

        TextView {
            text: "AI Demo"
            font.pixelSize: Resource.h1
            color: Resource.whiteColor
            Layout.alignment: Qt.AlignRight
        }
    }
}
