import QtQuick 2.0
// TBD
Item {
	id: root
	property double curr_ms: 0
	width: 800; height: 480
	Rectangle {
		width: 400; height: 480
		anchors.left: parent.left
		Animation2 {
			width: 400; height: 400
			anchors.centerIn: parent
		}
	}
	Rectangle {
		width: 400; height: 480
		anchors.right: parent.right
		Statistics{
			anchors.fill: parent
			anchors.centerIn: parent
		}
	}
}
