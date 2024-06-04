pragma Singleton

import QtQuick 2.15
import QtQuick.Window 2.1

QtObject {
    readonly property color background: "#FFFFFF"
    readonly property color baseColor: "#007dc3"
    readonly property color grayColor: "#494949"
    readonly property color whiteColor: "#FFFFFF"

    property int x : Window.width
    property int y : Screen.width

    property int leftPanelWidth : Screen.width/4
    property int leftPanelHeight : Screen.height - (Screen.height * 0.2)
    property int headerWidth : Screen.width
    property int headerHeight : (Screen.height/4) * 0.4
    property int rightPanelWidth : 3 * (Screen.width/4)
    property int rightPanelHeight : Screen.height - (Screen.height * 0.2)

    readonly property int h1: Math.round(Screen.width/30)
    readonly property int h2: Math.round(Screen.width/40)
    readonly property int h3: Math.round(Screen.width/60)
    readonly property int h4: Math.round(Screen.width/70)
}
