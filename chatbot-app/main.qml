import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: win
    visible: true
    width: 760; height: 720
    color: "#0E1220"
    title: "Local Chatbot — a real model on your machine"

    readonly property color card:   "#171B2C"
    readonly property color line:   "#262B40"
    readonly property color accent: "#7C5CFF"
    readonly property color muted:  "#8A8FA3"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---------- header ----------
        Rectangle {
            Layout.fillWidth: true; height: 62; color: card
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 18; anchors.rightMargin: 16; spacing: 10
                Rectangle { width: 30; height: 30; radius: 8; color: accent
                    Text { anchors.centerIn: parent; text: "🦙"; font.pixelSize: 16 } }
                ColumnLayout {
                    spacing: 0
                    Text { text: "Local Chatbot"; color: "white"; font.pixelSize: 17; font.bold: true }
                    Text { text: "Qwen 0.5B · running on your Mac · no cloud"; color: muted; font.pixelSize: 11 }
                }
                Item { Layout.fillWidth: true }
                Row {
                    spacing: 6
                    Rectangle { width: 9; height: 9; radius: 5; anchors.verticalCenter: parent.verticalCenter
                        color: chat.ready ? "#3FB37F" : "#E0B341" }
                    Text { text: chat.status; color: muted; font.pixelSize: 12
                           anchors.verticalCenter: parent.verticalCenter }
                }
                Rectangle {
                    width: 64; height: 30; radius: 8; color: clearMa.containsMouse ? "#20263C" : "transparent"
                    border.color: line
                    Text { anchors.centerIn: parent; text: "Clear"; color: muted; font.pixelSize: 12 }
                    MouseArea { id: clearMa; anchors.fill: parent; hoverEnabled: true; onClicked: chat.clearChat() }
                }
            }
        }

        // ---------- conversation ----------
        ListView {
            id: list
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            model: chat.messages
            spacing: 12
            topMargin: 18; bottomMargin: 18; leftMargin: 18; rightMargin: 18
            ScrollBar.vertical: ScrollBar { }
            onCountChanged: positionViewAtEnd()
            Component.onCompleted: positionViewAtEnd()

            delegate: Row {
                width: list.width - 36
                layoutDirection: modelData.role === 0 ? Qt.RightToLeft : Qt.LeftToRight
                Rectangle {
                    width: Math.min(bubble.implicitWidth + 28, list.width * 0.74)
                    height: bubble.implicitHeight + 22
                    radius: 14
                    color: modelData.role === 0 ? win.accent : win.card
                    border.color: modelData.role === 0 ? "transparent" : win.line
                    Text {
                        id: bubble
                        anchors { left: parent.left; right: parent.right; top: parent.top
                                  margins: 14 }
                        text: modelData.text
                        color: "white"; font.pixelSize: 15; wrapMode: Text.Wrap
                        textFormat: Text.PlainText
                    }
                }
            }

            // empty-state hint
            Text {
                anchors.centerIn: parent
                visible: list.count === 0
                width: parent.width * 0.7; horizontalAlignment: Text.AlignHCenter
                text: chat.ready ? "Ask it anything.\nIt's a small model — great for a chat, shaky on facts."
                                 : "Loading the model into memory…\n(first launch takes a few seconds)"
                color: muted; font.pixelSize: 14; wrapMode: Text.WordWrap
            }
        }

        // typing indicator
        Rectangle {
            Layout.fillWidth: true; height: chat.busy ? 26 : 0; color: "transparent"
            visible: chat.busy
            Text { anchors { left: parent.left; leftMargin: 24; verticalCenter: parent.verticalCenter }
                   text: "assistant is typing…"; color: muted; font.pixelSize: 12 }
        }

        // ---------- input ----------
        Rectangle {
            Layout.fillWidth: true; height: 70; color: card
            RowLayout {
                anchors.fill: parent; anchors.margins: 14; spacing: 10
                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; radius: 11
                    color: "#0E1524"; border.color: input.activeFocus ? accent : line
                    TextField {
                        id: input
                        anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        enabled: chat.ready && !chat.busy
                        placeholderText: chat.ready ? "Type a message and press Enter…" : "Waiting for the model…"
                        placeholderTextColor: "#5A6578"
                        color: "white"; font.pixelSize: 15; background: null
                        onAccepted: { chat.send(text); text = "" }
                    }
                }
                Rectangle {
                    width: 84; Layout.fillHeight: true; radius: 11
                    enabled: chat.ready && !chat.busy
                    opacity: (chat.ready && !chat.busy) ? 1 : 0.5
                    color: sendMa.pressed ? "#5B43D6" : (sendMa.containsMouse ? "#8B6BFF" : accent)
                    Text { anchors.centerIn: parent; text: "Send"; color: "white"; font.pixelSize: 15; font.bold: true }
                    MouseArea { id: sendMa; anchors.fill: parent; hoverEnabled: true
                        enabled: chat.ready && !chat.busy
                        onClicked: { chat.send(input.text); input.text = "" } }
                }
            }
        }
    }
}
