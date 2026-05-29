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

    Component.onCompleted: {
        astraVersionText.text = aboutManager.fetchAstraVersion();
        hwVersionText.text = aboutManager.fetchHwVersion();
        linuxVersionText.text = aboutManager.fetchLinuxVersion();
        ipAddress.text = aboutManager.fetchIpAddress();
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
            text: qsTr("About")
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
            text: qsTr("Astra Version")
            font.pixelSize: Resource.paragraphLarge
            isBold: true
            font.family: "Sans Serif"
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            id: astraVersionText
            text: about.astraVersion
            font.pixelSize: Resource.paragraphLarge
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("Hardware Version")
            font.pixelSize: Resource.paragraphLarge
            isBold: true
            font.family: "Sans Serif"
        }

        TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            id: hwVersionText
            text: about.hwVersion
            font.pixelSize: Resource.paragraphLarge
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }

        TextView {
            text: qsTr("Linux Build Version")
            font.pixelSize: Resource.paragraphLarge
            isBold: true
            font.family: "Sans Serif"
        }

         TextView {
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            id: linuxVersionText
            text: about.linuxVersion
            font.pixelSize: Resource.paragraphLarge
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }

        TextView {
            visible: ipAddress.text !== "Unknown IP" && ipAddress.text !== ""
            text: qsTr("IP Address")
            font.pixelSize: Resource.paragraphLarge
            isBold: true
            font.family: "Sans Serif"
        }

        TextView {
            visible: ipAddress.text !== "Unknown IP" && ipAddress.text !== ""
            text: ":"
            font.pixelSize: Resource.paragraphLarge
            font.family: "Sans Serif"
        }

        TextView {
            visible: ipAddress.text !== "Unknown IP" && ipAddress.text !== ""
            id: ipAddress
            text: about.ipAddress
            font.pixelSize: Resource.paragraphLarge
            font.capitalization: Font.MixedCase
            font.family: "Sans Serif"
        }
    }
}
