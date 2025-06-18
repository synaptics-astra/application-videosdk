import QtQuick 2.0
import QtQuick.Layouts 1.3

Item {
	id: root
	property double curr_ms: 0
	width: 800; height: 1280
	Rectangle {
		id: rect
		width: 800; height: 500
		anchors.horizontalCenter: parent.horizontalCenter
		Animation2 {
			width: 400; height: 400
			anchors.centerIn: parent
		}
	}

	Rectangle {
		id: rect2
		width: 800; height: 600
		anchors.top : rect.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		Statistics{
			anchors.fill: parent
		}
	}
}
