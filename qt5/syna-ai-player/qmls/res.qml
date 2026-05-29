pragma Singleton

import QtQuick 2.15
import QtQuick.Window 2.1

QtObject {
    readonly property color background: "#FFFFFF"
    readonly property color baseColor: "#007dc3"
    readonly property color grayColor: "#494949"
    readonly property color whiteColor: "#FFFFFF"

    property int width : 640
    property int height : 480

    property int leftPanelWidth : width/4
    property int leftPanelHeight : height - (height * 0.2)
    property int headerWidth : width
    property int headerHeight : (height/4) * 0.4
    property int rightPanelWidth : 3 * (width/4)
    property int rightPanelHeight : height - (height * 0.2)

    readonly property int h1: Math.round(width/30)
    readonly property int h2: Math.round(width/40)
    readonly property int h3: Math.round(width/60)
    readonly property int h4: Math.round(width/70)
}
