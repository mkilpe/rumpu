
import SPDrum 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Dialog {
	id: divisionDialog
	title: qsTr("Choose division for the measure")
	standardButtons: Dialog.Ok | Dialog.Cancel

	property alias model: barItem.model
	spacing: 5

	TextMetrics {
		id: textMetrics
		text: "0000000"
		font: barSpin.font
	}

	ColumnLayout {
		RowLayout {
			SpinBox {
				id: barSpin
				from: 1
				to: 32
				value: barItem.model.division()
				implicitWidth: textMetrics.width
				onValueModified: {
					barItem.model = barItem.model.divide(value);
					barItem.update();
				}
			}
			Bar {
				id: barItem
				width: 240
				height: 40
			}
		}
		RowLayout {
			Label {
				text: "Sub-Division, beat"
			}
			SpinBox {
				id: subDivSpins
				from: 1
				to: barSpin.value
				value: 1
				implicitWidth: textMetrics.width
			}
			ColumnLayout {
				Label {
					Layout.alignment: Qt.AlignHCenter
					text: subDivSlider.value
				}
				Slider {
					id: subDivSlider
					from: 1
					to: 16
					stepSize: 1
					value: barItem.model.subdivision(subDivSpins.value)
					onMoved: {
						barItem.model = barItem.model.subdivide(subDivSpins.value, value);
						barItem.update();
					}
				}
			}
		}
	}
}