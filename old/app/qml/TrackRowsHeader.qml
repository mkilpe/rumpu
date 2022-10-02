
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12


Column {
	id: rowsHeader

	property int infolength: 150
	property int defaultBarWidth: 120
	property int defaultBarHeight: 40

	z: 2
	spacing: 1

	Repeater {
		model: instrumentModel

		Rectangle {
			height: defaultBarHeight
			width: infolength

			color: "lightgreen"

			RowLayout {
				anchors {
					fill: parent
					margins: 2
				}
				Label {
					id: infoLabel
					Layout.alignment: Qt.AlignVCenter
					text: qsTr("⋮")
					font.pixelSize: defaultBarHeight/2
					verticalAlignment: Qt.AlignVCenter
				}
				Item {
					id: infoItem

					Layout.fillWidth: true
					Layout.fillHeight: true
					Label {
						id: nameLabel
						anchors {
							top: parent.top
							left: parent.left
							right: parent.right
						}
						height: parent.contentHeight/2
						text: name
						font.pixelSize: 13
						verticalAlignment: Qt.AlignVCenter
						elide: Text.ElideRight
					}

					RowLayout {
						anchors {
							top: nameLabel.bottom
							bottom: parent.bottom
							left: parent.left
							right: parent.right
						}
						Item {
							Layout.fillWidth: true
						}
						Dial {
							Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
							implicitHeight: 20
							implicitWidth: 20
							from: 0.0
							to: 1.0
							value: volume
							stepSize: 0.02
							snapMode: Dial.SnapAlways
							z: 10
							onMoved: volume = value
						}
						Button {
							Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
							implicitHeight: infoItem.height/2
							implicitWidth: infoItem.height/2

							font.pixelSize: 10
							text: mute ? qsTr("\u{1F509}") : qsTr("\u{1F507}")
							onClicked: mute = !mute
						}
					}
				}
			}
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

