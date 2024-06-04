import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import Resource 1.0
import "../customviews"

Pane {
    id: gridItem
    property int type: 0
    property string image: "qrc:/res/images/astraicon.png"
    property string title: qsTr("Syna AI Player")
    property string command1: "videotestsrc ! waylandsink"
    property string command2: "videotestsrc ! waylandsink"
    property string command3: "videotestsrc ! waylandsink"
    property string command4: "videotestsrc ! waylandsink"
    width: Resource.rightPanelWidth/2
    height: Resource.rightPanelWidth/2

    background: Rectangle {
        anchors.fill: parent
        color : "transparent"
        radius: 15
    }

    ColumnLayout {
        Image {
            source: gridItem.image
            sourceSize: Qt.size(Resource.rightPanelWidth/2, Resource.rightPanelWidth/2)
            MouseArea {
                width: parent.width
                height: parent.height
                focus: true
                anchors.fill: parent
                onClicked: {
                    switch (gridItem.type) {
                    case 1:
                        playbackControl.createManager()
                        playbackControl.doPlayback(gridItem.type, gridItem.command1, "", "", "")
                        break;
                    case 2:
                        playbackControl.createManager()
                        playbackControl.doPlayback(gridItem.type, gridItem.command1, gridItem.command2, gridItem.command3, gridItem.command4)
                        break;
                    default:
                        playbackControl.createManager()
                        playbackControl.doPlayback(1, gridItem.command1, gridItem.command2, gridItem.command3, gridItem.command4)
                        break;
                    }
                }
            }
        }

        TextView {
            text: title
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
