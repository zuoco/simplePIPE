import QtQuick
import QtQuick.Controls

Menu {
    id: root

    property bool hasSelection: false

    signal modifyRequested()
    signal viewRequested()
    signal deleteRequested()

    MenuItem {
        text: "修改"
        enabled: root.hasSelection
        onTriggered: root.modifyRequested()
    }

    MenuItem {
        text: "查看"
        enabled: root.hasSelection
        onTriggered: root.viewRequested()
    }

    MenuSeparator {}

    MenuItem {
        text: "删除"
        enabled: root.hasSelection
        onTriggered: root.deleteRequested()
    }
}
