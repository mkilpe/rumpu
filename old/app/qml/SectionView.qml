
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12



RowLayout {
	Label {
		text: qsTr("Section Order: ")
		font.pixelSize: 20
		Layout.topMargin: 2
	}

	ListView {
		id: sectionListView
		Layout.preferredHeight: 60
		Layout.fillWidth: true
		Layout.topMargin: 2

		orientation: Qt.Horizontal
		clip: true
		spacing: 1

		boundsBehavior: Flickable.StopAtBounds
		ScrollBar.horizontal: ScrollBar { active: true }

		model : sectionViewModel

		delegate: MouseArea {
			id: dragArea

			property bool held: false
			property int myindex: index

			anchors {
				verticalCenter: parent.verticalCenter
			}
			height: content.height
			width: content.width

			drag.target: held ? content : undefined
			drag.axis: Drag.XAxis

			pressAndHoldInterval: 200
			onPressAndHold: { 
				held = true;
				Drag.start();
			}
			onReleased: {
				Drag.drop();
				held = false;
			}

			onClicked: sectionListView.model.mouseClicked(index);

			Rectangle {
				id: content
				anchors {
					horizontalCenter: parent.horizontalCenter
					verticalCenter: parent.verticalCenter
				}

				//Drag.dragType: Drag.None
				//Drag.active: dragArea.held
				Drag.source: dragArea
				Drag.hotSpot.x: width / 2
				Drag.hotSpot.y: height / 2

				border.width: 1
				border.color: "#34a6ed"
				color: dragArea.held ? Qt.rgba(0, 0, 1, 0.2) : "#87c6ed"
				radius: 4
				border {
					width: 1
					color: "red"
				}

				states: State {
					when: dragArea.held

					ParentChange { target: content; parent: sectionListView }
					AnchorChanges {
						target: content
						anchors { horizontalCenter: undefined; verticalCenter: undefined }
					}
				}

				implicitWidth: sectionItem.width + 6
				implicitHeight: sectionListView.height

				Rectangle {
					id: sectionItem
					anchors {
						horizontalCenter: parent.horizontalCenter
						top: parent.top
						topMargin: 3
						bottomMargin: 3
					}

					TextMetrics {
						id: textMetrics
						font: sectionId.font
						text: "999"
					}
					width: 4+textMetrics.width + leftPadding + rightPadding
					height: textMetrics.height
					radius: 4
					color: "lightgrey"
					TextInput {
						id: sectionId
						anchors.fill: parent
						clip: true
						font.pixelSize: 20
						text: display
						padding: 2
						maximumLength: 3
						horizontalAlignment: TextInput.AlignHCenter
						verticalAlignment: TextInput.AlignVCenter
						validator: IntValidator{bottom: 0; top: 999;}

						onTextEdited: {
							display = text;
						}
					}
				}
			}
			DropArea {
				anchors { fill: parent }

				onDropped: {
					console.log("dropped " + dragArea.myindex);
					//sectionListView.model.move(drop.source.myindex, drop.target.dragArea.myindex);
				}
			}
		}
	}


	Button {
		implicitHeight: sectionListView.height*9/10
		implicitWidth: implicitHeight
		font.pixelSize: implicitHeight*2/3
		text: "+"
		onClicked: {
			sectionViewModel.append()
		}
	}

}