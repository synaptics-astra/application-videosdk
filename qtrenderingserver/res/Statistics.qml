import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 2.12


Item {
	id: statistics
	objectName: "statistics"
	anchors.centerIn: parent
	Rectangle {
		color: "blue"
		opacity: 0.3
		radius: 10
		border.width: 2
		border.color: "white"
		anchors.fill: parent
	}
	property var total_sessions: 5

	function initStatistics(total_sess)
	{
		console.log("total_sessions:",total_sess)
		total_sessions = total_sess
	}

	GridLayout {
		id: call_statistics_grid
		anchors.centerIn: parent
		columns: (total_sessions >=10) ? 8 : 4
		rows: 18
		Label {
			text: "Statistics"
			id:heading
			Layout.alignment: Qt.AlignCenter
			Layout.column:0
			Layout.columnSpan:(total_sessions >=10) ? 8 : 4
			Layout.row:0
			color: "White"
			font.bold: true
			font.pixelSize: 30
			font.underline: true
		}
		Label {
			text: "No"
			id:col1
			Layout.column:0
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
		}
		Label {
			text: "Type"
			id:col2
			Layout.column:1
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
		}

		Label {
			text: "Resolution"
			id:col3
			Layout.column:2
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
		}

		Label {
			text: "FPS"
			id:col4
			Layout.column:3
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
		}
		Label {
			text: "No"
			id:col5
			Layout.column:4
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
			visible:(total_sessions >=10) ? true : false
		}
		Label {
			text: "Type"
			id:col6
			Layout.column:5
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
			visible:(total_sessions >=10) ? true : false
		}

		Label {
			text: "Resolution"
			id:col7
			Layout.column:6
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
			visible:(total_sessions >=10) ? true : false
		}

		Label {
			text: "FPS"
			id:col8
			Layout.column:7
			Layout.row:1
			Layout.alignment: Qt.AlignLeft
			color: "White"
			font.bold: true
			font.pixelSize: 20
			visible:(total_sessions >=10) ? true : false
		}
		Repeater {
			model: total_sessions
			id: labelList
			Item {

				property alias inst_id_prop: inst_id.text
				property alias type_id_prop: type_id.text
				property alias res_id_prop: res_id.text
				property alias fps_id_prop: fps_id.text

				Label {
					parent:call_statistics_grid
					id: fps_id
					text: ""
					color: "White"
					font.bold: true
					Layout.fillWidth:true
					Layout.alignment: Qt.AlignCenter
					Layout.preferredWidth: 60
					background:Rectangle {
						width: fps_id.width-10
						height:1
						border.color: "white"
						color: "transparent"
						}
				}
				Label {
					parent:call_statistics_grid
					id: res_id
					text: ""
					color: "White"
					font.bold: true
					Layout.fillWidth:true
					Layout.preferredWidth: 150
					background:Rectangle {
						width: res_id.width+10
						height:1
						border.color: "white"
						color: "transparent"
						}
				}

				Label {
					parent:call_statistics_grid
					id: type_id
					text: ""
					color: "White"
					font.bold: true
					Layout.fillWidth:true
					Layout.preferredWidth: 60
					background:Rectangle {
						width: type_id.width+10
						height:1
						border.color: "white"
						color: "transparent"
						}
				}

				Label {
					parent:call_statistics_grid
					id: inst_id
					text: ""
					color: "White"
					font.bold: true
					Layout.fillWidth:true
					Layout.preferredWidth: 40
					Layout.alignment: Qt.AlignCenter
					background:Rectangle {
						width: inst_id.width+10
						height:1
						border.color: "white"
						color: "transparent"
						}
				}

			}
		}
	}

	function updateStatistics(inst,type,res,fps) {
					labelList.itemAt(inst-1).inst_id_prop = inst
					labelList.itemAt(inst-1).type_id_prop = type
					labelList.itemAt(inst-1).res_id_prop = res
					labelList.itemAt(inst-1).fps_id_prop = fps
	}

}


