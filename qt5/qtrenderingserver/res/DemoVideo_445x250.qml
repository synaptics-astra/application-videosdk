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
			x:28; y:16
			width: 445; height:250
			visible: false
			windowID: 0 // self
		}
		DemoVideoItem {
			id: screen1
			x:445+2*28; y:16
			width: 445; height:250
			visible: false
			windowID: 1 // self
		}
		DemoVideoItem {
			id: screen2
			x:445*2+3*28; y:16
			width: 445; height:250
			visible: false
			windowID: 2 // self
		}
		DemoVideoItem {
			id: screen3
			x:445*3+4*28; y:16
			width: 445; height:250
			visible: false
			windowID: 3 // self
		}
		DemoVideoItem {
			id: screen4
			x:28; y:282
			width: 445; height:250
			visible: false
			windowID: 4 // self
		}
		DemoVideoItem {
			id: screen5
			x:445+2*28; y:282
			width: 445; height:250
			visible: false
			windowID: 5 // self
		}
		DemoVideoItem {
			id: screen6
			x:445*2+3*28; y:282
			width: 445; height:250
			visible: false
			windowID: 6 // self
		}
		DemoVideoItem {
			id: screen7
			x:445*3+4*28; y:282
			width: 445; height:250
			visible: false
			windowID: 7 // self
		}
		DemoVideoItem {
			id: screen8
			x:28; y:548
			width: 445; height:250
			visible: false
			windowID: 8 // self
		}
		DemoVideoItem {
			id: screen9
			x:445+2*28; y:548
			width: 445; height:250
			visible: false
			windowID: 9 // self
		}
		DemoVideoItem {
			id: screen10
			x:445*2+3*28; y:548
			width: 445; height:250
			visible: false
			windowID: 10 // self
		}
		DemoVideoItem {
			id: screen11
			x:445*3+4*28; y:548
			width: 445; height:250
			visible: false
			windowID: 11 // self
		}
		DemoVideoItem {
			id: screen12
			x:28; y:814
			width: 445; height:250
			visible: false
			windowID: 12 // self
		}
		DemoVideoItem {
			id: screen13
			x:445+2*28; y:814
			width: 445; height:250
			visible: false
			windowID: 13 // self
		}
		DemoVideoItem {
			id: screen14
			x:445*2+3*28; y:814
			width: 445; height:250
			visible: false
			windowID: 14 // self
		}
		DemoVideoItem {
			id: screen15
			x:445*3+4*28; y:814
			width: 445; height:250
			visible: false
			windowID: 15 // self
		}
	}
}
