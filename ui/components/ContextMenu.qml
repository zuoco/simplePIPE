import QtQuick
import QtQuick.Controls

Menu {
    id: root

    signal inspectRequested()

    MenuItem {
        text: "查看属性"
        onTriggered: root.inspectRequested()
    }
}
