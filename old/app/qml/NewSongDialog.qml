
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3

// song name
// time signature
// tempo
//-- later
// accent pattern + volume
// random hit offset settings

Dialog {
	id: newSongDialog
	title: qsTr("New Song")
	standardButtons: Dialog.Ok | Dialog.Cancel

	property alias name: songName.text
	property alias timing_up: timingUp.value
	property alias timing_down: timingDown.currentText
	property alias tempo: tempoEdit.text

	TextMetrics {
		id: textMetrics
		text: "0000000"
		font: timingUp.font
	}

	function clear() {
		name = "";
	}

	GridLayout {
		columns: 2
		rows: 3

		Label {
			Layout.column: 0
			Layout.row: 0
			text: "Song Title"
		}

		Rectangle {
			Layout.column: 1
			Layout.row: 0
			width: textMetrics.width*6
			height: textMetrics.height
			color: "lightgrey"
			TextInput {
				id: songName
				anchors.fill: parent
				clip: true
			}
		}
		Label {
			Layout.column: 0
			Layout.row: 1
			text: "Time Signature"
		}
		ColumnLayout {
			Layout.column: 1
			Layout.row: 1
			Layout.alignment: Qt.AlignLeft
			SpinBox {
				id: timingUp
				implicitWidth: textMetrics.width
				//editable: true spinboxes require enter to change the entered value, this doesn't work too well in dialog
				from: 1
				to: 128
				value: 4
				clip: true
			}
			MenuSeparator {
				implicitWidth: textMetrics.width
			}
			ComboBox {
				id: timingDown
				implicitWidth: textMetrics.width
				model: [1, 2, 4, 8, 16, 32, 64, 128]
				currentIndex: 2
				clip: true
			}
		}
		Label {
			Layout.column: 0
			Layout.row: 2
			text: "Tempo"
		}
		RowLayout {
			Layout.column: 1
			Layout.row: 2
			Layout.alignment: Qt.AlignLeft


			Rectangle {
				TextMetrics {
					id: tempoTextMetrics
					text: "000"
					font: tempoEdit.font
				}
				radius: 4
				color: "lightgrey"
				width: tempoTextMetrics.width + 4
				height: tempoTextMetrics.height + 2
				TextInput {
					id: tempoEdit
					clip: true
					text: "120"
					maximumLength: 3
					horizontalAlignment: TextInput.AlignHCenter
					verticalAlignment: TextInput.AlignVCenter
					validator: IntValidator{bottom: 1; top: 999;}
				}
			}
/*
			SpinBox {
				id: tempoEdit
				implicitWidth: textMetrics.width
				editable: true
				from: 1
				to: 999
				value: 120
				clip: true
			}*/
			Label {
				Layout.alignment: Qt.AlignLeft
				text: "bpm"
			}
		}
	}
}