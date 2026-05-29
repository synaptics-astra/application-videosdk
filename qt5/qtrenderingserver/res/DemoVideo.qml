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
			id: screen13
			x:54; y:30
			width: 320*5 + 53*4; height:180*5 + 30*4
			visible: false
			windowID: 13 // self
		}
		DemoVideoItem {
			id: screen0
			x:54; y:30
			width: 320; height:180
			visible: false
			windowID: 0 // self
		}
		DemoVideoItem {
			id: screen1
			x:320+54+53; y:30
			width: 320; height:180
			visible: false
			windowID: 1 // self
		}
		DemoVideoItem {
			id: screen2
			x:320*2+54+53*2; y:30
			width: 320; height:180
			visible: false
			windowID: 2 // self
		}
		DemoVideoItem {
			id: screen3
			x:320*3+54+53*3; y:30
			width: 320; height:180
			visible: false
			windowID: 3 // self
		}
		DemoVideoItem {
			id: screen4
			x:320*4+54+53*4; y:30
			width: 320; height:180
			visible: false
			windowID: 4 // self
		}
		DemoVideoItem {
			id: screen5
			x:54; y:240
			width: 320; height:180
			visible: false
			windowID: 5 // self
		}
		DemoVideoItem {
			id: screen6
			x:320+54+53; y:240
			width: 320; height:180
			visible: false
			windowID: 6 // self
		}
		DemoVideoItem {
			id: screen7
			x:54; y:450
			width: 320; height:180
			visible: false
			windowID: 7 // self
		}
		DemoVideoItem {
			id: screen8
			x:320+54+53; y:450
			width: 320; height:180
			visible: false
			windowID: 8 // self
		}
		DemoVideoItem {
			id: screen9
			x:54; y:660
			width: 320; height:180
			visible: false
			windowID: 9 // self
		}
		DemoVideoItem {
			id: screen10
			x:320+54+53; y:660
			width: 320; height:180
			visible: false
			windowID: 10 // self
		}
		DemoVideoItem {
			id: screen11
			x:54; y:870
			width: 320; height:180
			visible: false
			windowID: 11 // self
		}
		DemoVideoItem {
			id: screen12
			x:320+54+53; y:870
			width: 320; height:180
			visible: false
			windowID: 12 // self
		}
	}
}
