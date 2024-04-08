import QtQuick 2.0
import DemoItem 1.0

Item {
	id: root
	property double curr_ms: 0
	Item {
		id: video_container
		objectName: "DemoVideoItem"
		anchors.fill: parent
		function qmlFunc(show) {
			console.log("Got message:", show)
			blur.alpha = show;
			return 0;
		}
		DemoVideoItem {
			id: screen0
			x:0; y:120
			width: 800; height:450
			visible: false
			windowID: 0 // self
		}
		DemoVideoItem {
			id: screen1
			x:0; y:120+450+140
			width: 800; height:450
			visible: false
			windowID: 1 // self
		}
	}
}
