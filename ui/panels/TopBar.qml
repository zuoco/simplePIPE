import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

Rectangle {
    id: root

    property var workbenchController
    property var appController
    property var onNewRequested
    property var onOpenRequested

    height: theme.topBarHeight
    color: "#FFFFFF"
    border.color: theme.divider
    border.width: 1

    Theme {
        id: theme
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 8

        Label {
            text: appController && appController.documentName.length > 0
                ? appController.documentName
                : "Untitled"
            font.pixelSize: theme.titleSize
            font.family: theme.fontFamily
            color: theme.textPrimary
            Layout.rightMargin: 8
        }

        ComboBox {
            id: wbCombo
            model: workbenchController ? workbenchController.workbenchNames : []
            currentIndex: {
                if (!workbenchController || !model || model.length <= 0) {
                    return -1
                }
                return Math.max(0, model.indexOf(workbenchController.activeWorkbench))
            }
            onActivated: {
                if (workbenchController) {
                    workbenchController.switchWorkbench(currentText)
                }
            }
            Layout.preferredWidth: 120
        }

        Rectangle {
            Layout.fillWidth: true
            color: "transparent"
            implicitHeight: 1
        }

        IconButton {
            iconText: "N"
            tooltip: "新建 (Ctrl+N)"
            onClicked: {
                if (typeof root.onNewRequested === "function") {
                    root.onNewRequested()
                }
            }
        }

        IconButton {
            iconText: "O"
            tooltip: "打开 (Ctrl+O)"
            onClicked: {
                if (typeof root.onOpenRequested === "function") {
                    root.onOpenRequested()
                }
            }
        }

        IconButton {
            iconText: "S"
            tooltip: "保存 (Ctrl+S)"
        }

        IconButton {
            iconText: "↶"
            tooltip: "撤销 (Ctrl+Z)"
            enabled: appController ? appController.canUndo : false
            onClicked: appController ? appController.undo() : undefined
        }

        IconButton {
            iconText: "↷"
            tooltip: "重做 (Ctrl+Y)"
            enabled: appController ? appController.canRedo : false
            onClicked: appController ? appController.redo() : undefined
        }
    }
}
