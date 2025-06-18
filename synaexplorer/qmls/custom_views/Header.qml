import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

Image{
    width: Resource.headerWidth
    height: Resource.headerHeight
    source: "qrc:/res/images/header_banner.png"
    TextView{
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        text: "Applications"
        font.pixelSize: Resource.h1
        color: Resource.whiteColor
    }
}
