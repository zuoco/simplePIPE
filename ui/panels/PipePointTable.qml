// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

CollapsiblePanel {
    id: root

    property var tableModel
    property bool editEnabled: true

    title: "管点表格"
    fillHeight: true

    Theme {
        id: theme
    }

    HorizontalHeaderView {
        id: header
        anchors.left: table.left
        anchors.top: parent.top
        syncView: table
        model: root.tableModel
    }

    TableView {
        id: table
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        clip: true
        rowSpacing: 1
        columnSpacing: 1
        model: root.tableModel

        selectionModel: ItemSelectionModel {
            model: table.model
        }

        delegate: EditableCell {
            required property int row
            required property int column

            text: model.display === undefined ? "" : String(model.display)
            editable: root.editEnabled
            selected: table.currentRow === row

            onEditCommitted: function(value) {
                if (!root.tableModel) {
                    return
                }
                const idx = root.tableModel.index(row, column)
                root.tableModel.setData(idx, value, 2)
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    table.currentRow = row
                    table.currentColumn = column
                    if (root.tableModel) {
                        root.tableModel.selectRow(row)
                    }
                }
            }
        }

        rowHeightProvider: function() {
            return theme.rowHeight
        }

        columnWidthProvider: function(col) {
            if (col === 0 || col === 5) {
                return 130
            }
            if (col === 4) {
                return 100
            }
            return 90
        }
    }
}
