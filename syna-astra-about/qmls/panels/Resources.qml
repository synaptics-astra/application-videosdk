import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0
import "../custom_views"

Rectangle {
    id: aboutRoot
    color: Resource.background

    Item {
        id: spacer_0
        height: Resource.width/50
        width: 1
    }

    ColumnLayout {
        id: header
        width: aboutRoot.width
        spacing: 20

        anchors.top: spacer_0.bottom

        TextView {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 100
            font.pixelSize: Resource.h2
            isBold: true
            isHighlight: true
            text: qsTr("Resources")
        }

        TextView {
            Layout.fillWidth: true
            Layout.leftMargin: 100
            Layout.rightMargin: 200
            Layout.margins: 20
            text: qsTr("For detailed information on the Astra, please refer the below mentioned links")
            wrapMode: Text.Wrap
            font.capitalization: Font.MixedCase
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }
    }

    GridLayout {
        anchors.left: parent.left
        anchors.top: header.bottom
        anchors.topMargin: 20
        anchors.leftMargin: 100
        anchors.margins: 20
        columns: 3
        columnSpacing: 10
        rowSpacing: 10


        TextView {
            text: qsTr("Homepage")
            font.pixelSize: Resource.paragraphLarge
            font.bold: true
            font.family: "Sans Serif"
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("Synaptics Astra Homepage")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            color: "blue"
            font.underline: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    ChromiumLauncher.openInChromium("https://www.synaptics.com/products/embedded-processors");
                }
            }
        }

        TextView {
            text: qsTr("Github")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            font.bold: true
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("Synaptics Astra Github")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            color: "blue"
            font.underline: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    ChromiumLauncher.openInChromium("https://github.com/synaptics-astra");
                }
            }
        }

        TextView {
            text: qsTr("Documentations")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            font.bold: true
        }

         TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("SDK Documentations")
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
            color: "blue"
            font.underline: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    ChromiumLauncher.openInChromium("https://synaptics-astra.github.io/doc/");
                }
            }
        }
    }
}
