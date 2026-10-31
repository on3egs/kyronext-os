// Original static artwork and layout: Kyronext Systems — By Manix.
import QtQuick

Rectangle {
    id: root
    color: "#101318"
    property int stage: 0

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 600)
        spacing: 18

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(root.width * 0.18, root.height * 0.20, 128)
            height: width
            source: "/usr/share/icons/hicolor/scalable/apps/kyronext-logo.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize.width: 256
            sourceSize.height: 256
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: "Kyronext OS"
            color: "#f0f2f7"
            font.family: "Noto Sans"
            font.pixelSize: Math.min(42, Math.max(22, root.width / 24))
            font.weight: Font.DemiBold
        }
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width * 0.60, 280)
            height: 2
            color: "#db2e46"
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.stage < 5 ? "Préparation du bureau…" : "Bienvenue"
            color: "#b8c0ce"
            font.pixelSize: 15
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: "Kyronext Systems — By Manix"
            color: "#8993a3"
            font.pixelSize: 12
        }
    }
}
