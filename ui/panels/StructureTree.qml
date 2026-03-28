import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

CollapsiblePanel {
    id: root

    property var treeModel

    title: "结构树"
    fillHeight: true

    Theme {
        id: theme
    }

    TreeView {
        id: treeView
        anchors.fill: parent
        clip: true
        model: root.treeModel

        delegate: Item {
            required property TreeView treeView
            required property bool hasChildren
            required property bool expanded
            required property int depth
            required property int row
            required property int column
            required property bool current
            required property var model

            implicitHeight: theme.rowHeight

            Rectangle {
                anchors.fill: parent
                color: current ? "#DCEEFF" : (hoverArea.containsMouse ? "#F6FAFE" : "transparent")
            }

            Row {
                anchors.fill: parent
                anchors.leftMargin: 6 + depth * 14
                spacing: 6

                Label {
                    width: 14
                    verticalAlignment: Text.AlignVCenter
                    text: hasChildren ? (expanded ? "▾" : "▸") : ""
                    color: theme.textSecondary
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.name
                    color: theme.textPrimary
                    elide: Text.ElideRight
                }
            }

            MouseArea {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    treeView.currentIndex = treeView.index(row, column)
                    if (root.treeModel && model.uuid && model.uuid.length > 0) {
                        root.treeModel.selectNodeByUuid(model.uuid)
                    }
                    if (hasChildren && mouse.x < (24 + depth * 14)) {
                        treeView.toggleExpanded(treeView.index(row, column))
                    }
                }
            }
        }
    }
}
