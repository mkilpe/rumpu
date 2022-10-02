
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import QtMultimedia 5.12

Dialog {
	id: sampleDialog
	title: qsTr("Select instrument to be used")
	standardButtons: Dialog.Ok | Dialog.Cancel

	property string url: ""

	function clear() {
		url = "";
	}

	onActionChosen: {
		if(action.button == StandardButton.Ok) {
			action.accepted = url != "";
		}
	}

	ColumnLayout {
		Button {
			text: qsTr("Load Audio File")
			onClicked: {
				selection.open();
			}

			FileDialog {
				id: selection
				title: qsTr("Choose audio file")
				folder: shortcuts.home
				defaultSuffix: qsTr("wav")
				nameFilters: [ qsTr("Waveform Audio File (*.wav)"), qsTr("All files (*)") ]

				onAccepted: {
					url = selection.fileUrl
				}
			}
		}
		RowLayout {
			spacing: 2
			Button {
				text: qsTr("\u25B6")
				Layout.preferredWidth: 20
				Layout.preferredHeight: 20
				onClicked: playSound.play()
				enabled: url != ""
			}
			Label {
				id: fileName
				text: url
				elide: Text.ElideLeft
				Layout.preferredWidth: 200
				Layout.fillWidth: true
			}
			SoundEffect {
				id: playSound
				source: url
			}
		}
	}
}