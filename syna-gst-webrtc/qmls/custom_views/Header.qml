import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

Image {
    width: Resource.headerWidth
    height: Resource.headerHeight
    source: "qrc:/res/images/header_banner.png"

    RowLayout {
        width: parent.width - 20
        height: parent.height
        x: 10
        Image {
            source: "qrc:/res/images/astra_icon.png"
            sourceSize: Qt.size(parent.height, parent.height)
            Layout.preferredWidth: parent.height
            Layout.preferredHeight: parent.height
        }

        TextView {
            text: "Real time streaming"
            font.pixelSize: Resource.h1
            color: Resource.whiteColor
            Layout.alignment: Qt.AlignRight
        }
    }
}
