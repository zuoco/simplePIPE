// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

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

    Connections {
        target: appController
        function onInsertComponentRequested(componentType) {
            var labels = {
                "insert-pipe":           "直管段",
                "insert-elbow":          "弯头",
                "insert-tee":            "三通",
                "insert-reducer":        "大小头",
                "insert-valve":          "阀门",
                "insert-flange":         "法兰",
                "insert-rigid-support":  "刚性支撑",
                "insert-spring-hanger":  "弹簧支吊架",
                "insert-guide":          "导向约束",
                "insert-restraint":      "位移限位器",
                "insert-beam":           "结构梁"
            }
            var label = labels[componentType] || componentType
            statusToast.show("插入 " + label + " — 请在视口中点击管点")
        }
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
        onActivated: {
            if (appController && appController.hasSelection) {
                deleteConfirmDialog.open()
            }
        }
    }

    // Delete confirmation dialog
    Dialog {
        id: deleteConfirmDialog
        title: "确认删除"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel

        Label {
            text: "确定要删除选中的对象吗？"
        }

        onAccepted: {
            if (appController) {
                appController.deleteSelected()
                statusToast.show("已删除选中对象")
            }
        }
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
                        } else if (modelData === "VtkViewport") {
                            item.objectName = "vtkViewportPanel"
                            item.appController = Qt.binding(function() { return appController })
                            if (workbenchController) workbenchController.notifyViewportLoaded(item.vtkViewport);

                        } else if (modelData === "Viewport3D") {
                            item.objectName = "viewportPanel"
                            item.appController = Qt.binding(function() { return appController })
                            item.inspectRequested.connect(root.showPropertyPanelHint)
                            item.modifyRequested.connect(function() {
                                if (appController) {
                                    appController.editModeRequested()
                                    root.showPropertyPanelHint()
                                }
                            })
                            item.viewModeRequested.connect(function() {
                                if (appController) {
                                    appController.viewModeRequested()
                                    root.showPropertyPanelHint()
                                }
                            })
                            item.deleteRequested.connect(function() {
                                if (appController && appController.hasSelection) {
                                    deleteConfirmDialog.open()
                                }
                            })
                            item.boxSelectFinished.connect(function(sx, sy, ex, ey, append) {
                                // Box selection results are handled by C++ pick handler
                                // QML signals the coordinates; actual selection is TBD when C++ bridge is wired
                                statusToast.show("框选完成")
                            })
                            if (workbenchController) workbenchController.notifyViewportLoaded(item.vsgViewport);
                        } else if (modelData === "ParameterPanel") {
                            item.objectName = "parameterPanel"
                            item.tableModel = Qt.binding(function() { return appController ? appController.pipePointTableModel : null })
                            item.propertyModel = Qt.binding(function() { return appController ? appController.propertyModel : null })
                        } else if (modelData === "ComponentToolStrip") {
                            item.objectName = "componentToolStripPanel"
                            item.appController = Qt.binding(function() { return appController })
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
