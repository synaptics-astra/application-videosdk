import QtQuick 2.0

Rectangle {
	id: root
	width: 1280
	height: 800
	x: -240
	y: 240
	rotation: 90
	color: "white"

	Text {
		id: synaveda
		z: 3
		anchors.centerIn: parent
		color: "blue"
		text: "SynaAVeda"
		font.pointSize: 40

		NumberAnimation on rotation { from:0; to: 360; duration: 100000; loops: Animation.Infinite }
		ColorAnimation on color { from:"blue"; to: "red"; duration: 1000; loops: Animation.Infinite }
	}

	Text {
		z: 3
		anchors.horizontalCenter: synaveda.horizontalCenter
		anchors.top: synaveda.bottom
		anchors.topMargin: 20
		color: "red"
		text: "Synaptics Audio Video Development (Demo) Applications..!"
		font.pointSize: 15

		ColorAnimation on color { from:"red"; to: "blue"; duration: 1000; loops: Animation.Infinite }
	}
}


