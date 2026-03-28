import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "dialogs"
import "panels"
import "style"

ApplicationWindow {
    id: root

    width: 1480
    height: 900
    visible: true
    color: theme.bgPrimary
    title: (appController && appController.documentName.length > 0 ? appController.documentName : "Untitled") + " - PipeCAD"

    Theme {
        id: theme
    }

    function showPropertyPanelHint() {
        propertyPanel.ensureExpandedAndFlash()
    }

    Shortcut {
        sequence: StandardKey.Undo
        onActivated: if (appController) appController.undo()
    }

    Shortcut {
        sequence: StandardKey.Redo
        onActivated: if (appController) appController.redo()
    }

    Shortcut {
        sequence: "Ctrl+S"
        onActivated: statusToast.show("已触发保存快捷键")
    }

    Shortcut {
        sequence: "Ctrl+N"
        onActivated: newProjectDialog.open()
    }

    Shortcut {
        sequence: "Ctrl+O"
        onActivated: openProjectDialog.open()
    }

    Shortcut {
        sequence: "Delete"
        onActivated: statusToast.show("已触发删除快捷键")
    }

    NewProjectDialog {
        id: newProjectDialog
        onAccepted: {
            if (appController) {
                appController.documentName = projectName.length > 0 ? projectName : "Untitled"
            }
            statusToast.show("已创建新工程")
        }
    }

    OpenProjectDialog {
        id: openProjectDialog
        onAccepted: {
            statusToast.show("打开工程: " + filePath)
        }
    }

    Popup {
        id: statusToast
        x: root.width - width - 20
        y: root.height - height - 44
        width: Math.max(160, toastLabel.implicitWidth + 18)
        height: 34
        padding: 0
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            radius: 4
            color: "#2D3A45"
            opacity: 0.9
        }

        Label {
            id: toastLabel
            anchors.centerIn: parent
            color: "#FFFFFF"
            text: ""
            font.pixelSize: 12
        }

        function show(message) {
            toastLabel.text = message
            open()
            toastTimer.restart()
        }

        Timer {
            id: toastTimer
            interval: 1400
            repeat: false
            onTriggered: statusToast.close()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            id: topBar
            Layout.fillWidth: true
            workbenchController: workbenchController
            appController: appController
            onNewRequested: function() { newProjectDialog.open() }
            onOpenRequested: function() { openProjectDialog.open() }
        }

        SplitView {
            id: horizontalSplit
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            StructureTree {
                id: structureTree
                objectName: "structureTreePanel"
                treeModel: appController ? appController.segmentTreeModel : null
                SplitView.minimumWidth: 200
                SplitView.preferredWidth: 260
            }

            Viewport3D {
                id: viewportPanel
                objectName: "viewportPanel"
                SplitView.minimumWidth: 400
                SplitView.preferredWidth: 860
                SplitView.fillWidth: true
                onInspectRequested: root.showPropertyPanelHint()
            }

            SplitView {
                id: rightSplit
                orientation: Qt.Vertical
                SplitView.minimumWidth: 320
                SplitView.preferredWidth: 380

                PipePointTable {
                    id: pipePointTable
                    objectName: "pipePointTablePanel"
                    tableModel: appController ? appController.pipePointTableModel : null
                    SplitView.minimumHeight: 220
                    SplitView.preferredHeight: 360
                }

                PropertyPanel {
                    id: propertyPanel
                    objectName: "propertyPanel"
                    propertyModel: appController ? appController.propertyModel : null
                    SplitView.minimumHeight: 220
                    SplitView.fillHeight: true
                }
            }
        }

        StatusBar {
            id: statusBar
            objectName: "statusBarPanel"
            Layout.fillWidth: true
            appController: appController
        }
    }
}
