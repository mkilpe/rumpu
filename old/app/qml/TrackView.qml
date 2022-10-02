
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Item {
	id: trackView

	ToolBar {
		id: trackTools
		anchors.left: parent.left
		anchors.right: parent.right

		visible: trackListModel.section > 0
		RowLayout {
			anchors.fill: parent

			Label {
				text: "Section " + trackListModel.section
			}

			ToolSeparator { Layout.fillHeight: true }

			RowLayout {
				Label {
					text: qsTr("Bars:")
				}
				SpinBox {
					id: bars
					from: 1
					to: 9999
					editable: true
					value: trackListModel.length
					onValueModified: {
						trackListModel.length = value;
					}
					TextMetrics {
						id: barsTextMetrics
						font: bars.font
						text: "9999"
					}
					implicitWidth: barsTextMetrics.width + leftPadding + rightPadding
				}
			}

			ToolSeparator { Layout.fillHeight: true }

			Item {
				Layout.fillWidth: true
			}
		}
	}

	TrackList {
		id: trackList
		model: trackListModel
		anchors {
			top: trackTools.bottom
			bottom: parent.bottom
			left: parent.left
			right: parent.right
			topMargin: 1
		}
	}
}
