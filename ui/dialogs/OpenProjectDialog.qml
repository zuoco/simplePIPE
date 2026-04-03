// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    modal: true
    title: "打开工程"
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias filePath: filePathField.text

    contentItem: ColumnLayout {
        spacing: 8

        Label {
            text: "文件路径"
        }

        TextField {
            id: filePathField
            Layout.fillWidth: true
            placeholderText: "/path/to/project.json"
            selectByMouse: true
        }
    }
}
