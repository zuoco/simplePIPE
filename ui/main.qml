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
        parameterPanel.ensurePropertyPanelVisibleAndFlash()
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

            Repeater {
                model: workbenchController ? workbenchController.activePanels : []
                delegate: Loader {
                    id: panelLoader
                    source: "panels/" + modelData + ".qml"
                    
                    SplitView.minimumWidth: item && item.SplitView ? item.SplitView.minimumWidth : 200
                    SplitView.preferredWidth: item && item.SplitView ? item.SplitView.preferredWidth : 300
                    SplitView.fillWidth: modelData === "Viewport3D"
                    
                    onLoaded: {
                        if (modelData === "DesignTree") {
                            item.objectName = "designTreePanel"
                            item.treeModel = Qt.binding(function() { return appController ? appController.segmentTreeModel : null })
                        } else if (modelData === "Viewport3D") {
                            item.objectName = "viewportPanel"
                            item.inspectRequested.connect(root.showPropertyPanelHint)
                            var vsg = item.children[1] // The second child is VsgViewport? Wait, it's safer to find it. But QML doesn't have findChild inside Loader easily without objectName and looping.
                            // Actually it's exposed as item.findChild in QtQuick since 5.9 maybe?
                            // Wait, we can just write a function in Viewport3D.qml to emit it.
                            if (workbenchController) workbenchController.notifyViewportLoaded(item.vsgViewport);
                        } else if (modelData === "ParameterPanel") {
                            item.objectName = "parameterPanel"
                            item.tableModel = Qt.binding(function() { return appController ? appController.pipePointTableModel : null })
                            item.propertyModel = Qt.binding(function() { return appController ? appController.propertyModel : null })
                        } else if (modelData === "PropertyPanel") {
                            item.objectName = "propertyPanel"
                            item.propertyModel = Qt.binding(function() { return appController ? appController.propertyModel : null })
                        }
                    }
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
