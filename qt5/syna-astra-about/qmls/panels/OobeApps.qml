import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import "../custom_views"

Rectangle {
    id: oobeRoot
    color: Resource.background
    property string boardName: ""

    Item {
        id: spacer_0
        height: Resource.width/50
        width: 1
    }

    Component.onCompleted: {
        boardName = aboutManager.getMachine();
    }

    ColumnLayout {
        id: header
        width: oobeRoot.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isHighlight: true
            text: qsTr("OOBE Applications")
        }

        TextView {
            Layout.fillWidth: true
            Layout.leftMargin: 100
            Layout.rightMargin: 200
            Layout.margins: 20
            font.pixelSize: Resource.paragraphLarge
            text: qsTr("Astra OOBE image includes built-in applications that demonstrate the platform's performance and capabilities.")
            wrapMode: Text.Wrap
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }
    }

    GridLayout {
        id : grid1
        anchors.left: parent.left
        anchors.top: header.bottom
        anchors.topMargin: 20
        anchors.leftMargin: 100
        anchors.margins: 20
        columns: 3
        columnSpacing: 10
        rowSpacing: 10

        TextView {
            text: qsTr("Chromium")
            font.pixelSize: Resource.paragraphLarge
            color: "blue"
            font.underline: true
            font.family: "Sans Serif"

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    ChromiumLauncher.openInChromium();
                }
            }
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("Embedded Chromium in Yocto enables lightweight browser integration for resource-constrained devices.")
            font.pixelSize: Resource.paragraphLarge
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("SynaExplorer")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            color: "blue"
            font.underline: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    SynaexplorerLauncher.openSynaexplorer(boardName);
                }
            }
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("Provides seamless access to featured app demonstrations.")
            font.pixelSize: Resource.paragraphLarge
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }

        TextView {
            visible: HAVE_WAYLAND
            text: qsTr("Wayland Terminal")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            color: "blue"
            font.underline: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    TerminalLauncher.openTerminalLauncher();
                }
            }
        }

        TextView {
            visible: HAVE_X11
            text: qsTr("XFCE Terminal")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            color: "blue"
            font.underline: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    TerminalLauncher.openTerminalLauncher();
                }
            }
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            visible: HAVE_WAYLAND
            text: qsTr("A fast and responsive terminal built for Wayland compositors.")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            font.capitalization: Font.MixedCase
        }

        TextView {
            visible: HAVE_X11
            text: qsTr("A fast and responsive terminal XFCE Terminal.")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            font.capitalization: Font.MixedCase
        }
    }
}
