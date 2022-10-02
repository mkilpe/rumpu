
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12


Row {
	id: header

	property int infolength: 150
	property int defaultBarWidth: 120
	property int defaultBarHeight: 40
	property int columns: 0

	z: 2

	Repeater {
		model: columns

		Rectangle {
			height: defaultBarHeight
			width: defaultBarWidth

			color: "white"

			MouseArea {
				anchors.fill: parent
				acceptedButtons: Qt.RightButton

				onClicked: {
					contextMenu.open();
				}
			}
			Menu {
				id: contextMenu
				MenuItem {
					text: qsTr("Details...")
					onTriggered: {
						//divDialog.open();
					}
				}
			}
		}
	}
}

