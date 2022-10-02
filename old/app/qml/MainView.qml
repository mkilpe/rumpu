import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {
	header: ToolBar {
		RowLayout {
			anchors.fill: parent
			ToolButton {
				Layout.alignment: Qt.AlignVCenter
				font.pixelSize: 20
				text: qsTr("+")
				enabled: trackListModel.section > 0
				onClicked: {
					selection.clear();
					selection.open();
				}

				LoadInstrument {
					id: selection
					onAccepted: {
						if(selection.url != "") {
							instrumentModel.add(selection.url);
						}
					}
				}
			}

			ToolSeparator {
				Layout.fillHeight: true
			}

			PlayToolBar {
				id: playToolBar
				Layout.alignment: Qt.AlignVCenter
				gain: mainControl.gain
				Binding {
					target: mainControl
					property: "gain"
					value: playToolBar.gain
				}
			}

			Item {
				Layout.fillWidth: true
			}
		}
	}

	ColumnLayout {
		anchors.fill: parent

		SectionView {
			Layout.fillWidth: true
		}

		TrackView {
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	}

	footer: RowLayout {
		Item {
			Layout.fillWidth: true
		}
		Label {
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
			text: "Time: " + mainControl.playTime.toFixed(3) + "s"
		}
		ToolSeparator {}
		Label {
			property int bar: mainControl.totalPlayBars > 0 ? mainControl.playBar+1 : 0
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
			text: "Bar: " + bar + "/" + mainControl.totalPlayBars
		}
		Item {
			Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
			width: 5
		}
	}

	Component.onCompleted: {
	}
}