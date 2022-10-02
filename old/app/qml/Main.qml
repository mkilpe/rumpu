import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3

ApplicationWindow {

	id: root
	width: 1000
	height: 650
	visible: true

	title: "SPDrum" + (mainControl.songName != "" ? (" - " + mainControl.songName) : "")

	Loader {
		id: mainContainer
		anchors.fill: parent
	}

	Component.onCompleted: {
		mainContainer.source = "/qml/MainView.qml";
	}

	menuBar : MenuBar {
		Menu {
			title: qsTr("&File")
			Action {
				text: qsTr("&New...")
				onTriggered: {
					newSongDialog.clear();
					newSongDialog.open();
				}
			}
			Action {
				text: qsTr("&Open...")
				onTriggered: openSongDialog.open();
			}
			Action {
				text: qsTr("&Save")
				onTriggered: mainControl.filename() != "" ? mainControl.save_song() : saveSongDialog.open();
			}
			Action {
				text: qsTr("Save &As...")
				onTriggered: saveSongDialog.open();
			}
			MenuSeparator { }
			Action {
				text: qsTr("&Export...")
				onTriggered: {
					exportDialog.open();
				}

			}
			MenuSeparator { }
			Action {
				text: qsTr("&Quit")
				onTriggered: Qt.quit()
			}
		}
		Menu {
			title: qsTr("&Edit")
			Action { text: qsTr("Cu&t") }
			Action { text: qsTr("&Copy") }
			Action { text: qsTr("&Paste") }
		}
		Menu {
			title: qsTr("&Help")
			Action {
				text: qsTr("&About")
				onTriggered: aboutDialog.open()
			}
		}
	}
	AboutDialog {
		id: aboutDialog
		x: (parent.width - width) / 2
		y: (parent.height - height) / 2
	}
	FileDialog {
		id: exportDialog
		title: qsTr("Export to wav file")
		folder: shortcuts.home
		defaultSuffix: "wav"
		nameFilters: [ qsTr("Waveform Audio File (*.wav)"), qsTr("All files (*)") ]
		selectExisting: false

		onAccepted: mainControl.export_song(exportDialog.fileUrl);
	}
	NewSongDialog {
		id: newSongDialog
		onAccepted: mainControl.new_song(name, timing_up, timing_down, tempo);
	}
	FileDialog {
		id: openSongDialog
		title: qsTr("Open Song")
		folder: shortcuts.home
		defaultSuffix: "spd"
		nameFilters: [ qsTr("Secure Path Drum (*.spd)"), qsTr("All files (*)") ]

		onAccepted: mainControl.load_song(openSongDialog.fileUrl);
	}
	FileDialog {
		id: saveSongDialog
		title: qsTr("Save Song")
		folder: shortcuts.home
		defaultSuffix: "spd"
		nameFilters: [ qsTr("Secure Path Drum (*.spd)"), qsTr("All files (*)") ]
		selectExisting: false

		onAccepted: mainControl.save_song(saveSongDialog.fileUrl);
	}
}