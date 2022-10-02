
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

TableView {
	id: view
	clip: true
	rowSpacing: 1
	reuseItems: false

	visible: trackListModel.section > 0

	property int infolength: 150
	property int defaultBarWidth: 120
	property int defaultBarHeight: 40

	leftMargin: infolength + 5
	topMargin: defaultBarHeight

	TrackHeader {
		id: trackHeader
		y: view.contentY

		infolength: view.infolength
		defaultBarWidth: view.defaultBarWidth
		defaultBarHeight: view.defaultBarHeight
		columns: view.rows > 0 ? (view.columns > 0 ? view.columns : 0) : 0
	}

	TrackRowsHeader {
		id: trackRowsHeader
		x: view.contentX

		infolength: view.infolength
		defaultBarWidth: view.defaultBarWidth
		defaultBarHeight: view.defaultBarHeight
	}


	delegate: Rectangle {
		implicitWidth: defaultBarWidth*beatZoom
		implicitHeight: defaultBarHeight

		color: has_selection(row, column) ? Qt.rgba(0.40, 0.40, 1.00, 0.50) : "white"

		Bar {
			id: barItem
			anchors.fill: parent
			model: bar
		}

		function update_bar(value) {
			view.model.setMark(row, column, value);
			barItem.update();
		}

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton | Qt.RightButton

			onClicked: {
				if(mouse.button == Qt.LeftButton) {
					if (mouse.modifiers & Qt.ControlModifier) {
						handle_select_click(row, column);
					} else {
						update_bar(bar.toggle_hit(mouse.x/width));
					}
				} else {
					contextMenu.popup();
				}
			}
			pressAndHoldInterval: 200
			onPressAndHold: {
				if(mouse.button == Qt.LeftButton) {
					update_bar(bar.toggle_stop(mouse.x/width));
				}
			}

			Menu {
				id: contextMenu
				MenuItem {
					text: qsTr("Division...")
					onTriggered: {
						divDialog.open();
					}
					DivisionDialog {
						id: divDialog
						model: bar
						onAccepted: update_bar(model)
					}
				}
				MenuSeparator {}
				Menu {
					title: qsTr("Clear")
					MenuItem {
						text: qsTr("Data")
						onTriggered: update_bar(bar.clear(true))
					}
					MenuItem {
						text: qsTr("Division and data")
						onTriggered: update_bar(bar.clear(false))
					}
				}
			}
		}
	}

	boundsBehavior: Flickable.StopAtBounds
	flickableDirection: Flickable.HorizontalAndVerticalFlick
	ScrollBar.vertical: ScrollBar { active: true }
	ScrollBar.horizontal: ScrollBar { active: true }

	property real beatZoom: 1.0

	MouseArea {
		anchors.fill: parent
		propagateComposedEvents: true
		onWheel: {
			if (wheel.modifiers & Qt.ControlModifier) {
				beatZoom += wheel.angleDelta.y / 360.0;
				beatZoom = Math.min(Math.max(beatZoom, 0.3), 50.0);
				wheel.accepted = true;

				//resetting the model seems to be the only way to redraw the canvas delegates correctly
				var model = view.model;
				view.model = 0;
				view.model = model;
			} else {
				wheel.accepted = false;
			}
		}
	}

	//focus: true
	/*Keys.onPressed: {
		console.log("testest");
		unselect();
	}*/

	function unselect() {
		hasSelection = -1;
		selectedRow = -1;
		selectedStart = -1;
		selectedEnd = -1;
	}

	function handle_select_click(row, column) {
		if(hasSelection == -1) {
			if(selectedRow == -1) {
				selectedRow = row;
				selectedStart = column;
			} else {
				if(selectedRow == row) {
					selectedEnd = column
					if(selectedEnd < selectedStart) {
						var tmp = selectedStart;
						selectedStart = selectedEnd;
						selectedEnd = tmp;
					}
					hasSelection = 1;
				} else {
					unselect();
				}
			}
		} else {
			unselect();
			selectedRow = row;
			selectedStart = column;
		}
	}

	function has_selection(row, column) {
		return hasSelection != -1 && selectedRow == row && selectedStart <= column && column <= selectedEnd;
	}

	property int hasSelection: -1
	property int selectedRow: -1
	property int selectedStart: -1
	property int selectedEnd: -1
}

