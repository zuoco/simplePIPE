import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeCAD 1.0

ApplicationWindow {
    id: root
    width: 1400
    height: 860
    visible: true
    title: appController.documentName.length > 0 ? appController.documentName : "PipeCAD"

    header: ToolBar {
        contentItem: RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 10

            Label {
                text: "Workbench"
                Layout.alignment: Qt.AlignVCenter
            }

            ComboBox {
                id: wbCombo
                model: workbenchController.workbenchNames
                currentIndex: Math.max(0, model.indexOf(workbenchController.activeWorkbench))
                onActivated: workbenchController.switchWorkbench(currentText)
            }

            Rectangle {
                Layout.fillWidth: true
                color: "transparent"
                implicitHeight: 1
            }

            Repeater {
                model: workbenchController.toolbarActions
                delegate: Button {
                    text: modelData
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            radius: 4
            color: "#F4F4F4"

            Text {
                anchors.centerIn: parent
                text: "Active Panels: " + workbenchController.activePanels.join(", ")
                color: "#3A3A3A"
                font.pixelSize: 13
            }
        }

        VsgViewport {
            id: viewport
            objectName: "viewport3d"
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
        }

        Frame {
            Layout.fillWidth: true
            padding: 10

            RowLayout {
                anchors.fill: parent
                spacing: 12

                Label {
                    text: "Selected: " + appController.selectedCount
                }

                Button {
                    text: "Clear Selection"
                    onClicked: appController.clearSelection()
                }

                Button {
                    text: "Undo"
                    enabled: appController.canUndo
                    onClicked: appController.undo()
                }

                Button {
                    text: "Redo"
                    enabled: appController.canRedo
                    onClicked: appController.redo()
                }
            }
        }
    }
}
