
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

RowLayout {
	property alias gain: gainDial.value

	ToolButton {
		Layout.alignment: Qt.AlignVCenter
		id: playButton
		font.pixelSize: 20
		text: qsTr("\u25B6")
		onClicked: mainControl.play()
	}
	ToolButton {
		Layout.alignment: Qt.AlignVCenter
		font: playButton.font
		text: qsTr("\u25FC")
		onClicked: mainControl.stop()
	}
	Item {
		Layout.alignment: Qt.AlignVCenter
		implicitHeight: playButton.height
		implicitWidth: playButton.height
		Dial {
			implicitHeight: parent.implicitHeight - gainLabel.implicitHeight
			implicitWidth: parent.width
			id: gainDial
			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
			}
			from: 0.0
			to: 1.5
			stepSize: 0.02
			snapMode: Dial.SnapAlways
			z: 10
		}
		Label {
			id: gainLabel
			font.pixelSize: 9
			anchors {
				top: gainDial.bottom
				bottom: parent.bottom
				horizontalCenter: parent.horizontalCenter
			}
			text: "Gain"
		}
	}
}