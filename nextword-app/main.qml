import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: win
    visible: true
    width: 1000; height: 680
    color: "#0E1220"
    title: "Guess the Next Word — a tiny AI you can watch"

    readonly property color card:   "#171B2C"
    readonly property color line:   "#262B40"
    readonly property color accent: "#7C5CFF"
    readonly property color muted:  "#8A8FA3"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 16

        // ---------- header ----------
        RowLayout {
            spacing: 12
            Rectangle { width: 34; height: 34; radius: 9; color: accent
                Text { anchors.centerIn: parent; text: "✎"; color: "white"; font.pixelSize: 19; font.bold: true } }
            ColumnLayout {
                spacing: 0
                Text { text: "Guess the Next Word"; color: "white"; font.pixelSize: 22; font.bold: true }
                Text { text: "A little language model writing one word at a time — watch it think."
                       color: muted; font.pixelSize: 13 }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: brain.transitions + " patterns · " + brain.vocabulary + " words · stored in SQLite"
                color: muted; font.pixelSize: 12
            }
        }

        // ---------- main area ----------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // ----- left: the writing -----
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 3; Layout.fillWidth: true
                radius: 14; color: card; border.color: line

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 10
                    Text { text: "It writes:"; color: muted; font.pixelSize: 13 }
                    Flickable {
                        id: flick
                        Layout.fillWidth: true; Layout.fillHeight: true
                        contentHeight: story.paintedHeight; clip: true
                        Text {
                            id: story
                            width: flick.width
                            text: brain.text.length ? brain.text : "Press Play to begin…"
                            color: brain.text.length ? "white" : muted
                            font.pixelSize: 21; lineHeight: 1.35; wrapMode: Text.WordWrap
                        }
                        onContentHeightChanged: contentY = Math.max(0, contentHeight - height)
                    }
                    Row {
                        spacing: 6
                        Rectangle { width: 9; height: 9; radius: 5; color: accent; anchors.verticalCenter: parent.verticalCenter
                                    SequentialAnimation on opacity { running: brain.running; loops: Animation.Infinite
                                        NumberAnimation { to: 0.2; duration: 400 } NumberAnimation { to: 1; duration: 400 } } }
                        Text { text: brain.running ? "writing…" : "paused"; color: muted; font.pixelSize: 12 }
                    }
                }
            }

            // ----- right: the thinking -----
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 2; Layout.fillWidth: true
                radius: 14; color: card; border.color: line

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 10
                    Text { text: "Choosing the word after:"; color: muted; font.pixelSize: 13 }
                    Text { text: brain.context; color: "white"; font.pixelSize: 16; font.bold: true
                           elide: Text.ElideRight; Layout.fillWidth: true }
                    Rectangle { Layout.fillWidth: true; height: 1; color: line }

                    // candidate bars
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8
                        Repeater {
                            model: brain.candidates
                            delegate: Item {
                                Layout.fillWidth: true
                                height: 30
                                Text {
                                    id: wlabel
                                    width: 92
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.word
                                    color: modelData.chosen ? "white" : muted
                                    font.pixelSize: 14; font.bold: modelData.chosen
                                    elide: Text.ElideRight
                                }
                                Rectangle {   // track
                                    anchors { left: wlabel.right; leftMargin: 8; right: pct.left; rightMargin: 8
                                              verticalCenter: parent.verticalCenter }
                                    height: 16; radius: 8; color: "#10152420" ; border.color: line
                                    Rectangle {   // fill
                                        height: parent.height; radius: 8
                                        width: Math.max(6, parent.width * modelData.prob)
                                        color: modelData.chosen ? accent : "#3A4160"
                                        Behavior on width { NumberAnimation { duration: 260; easing.type: Easing.OutCubic } }
                                        Behavior on color { ColorAnimation { duration: 200 } }
                                    }
                                }
                                Text {
                                    id: pct
                                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                                    width: 42; horizontalAlignment: Text.AlignRight
                                    text: Math.round(modelData.prob * 100) + "%"
                                    color: modelData.chosen ? "white" : muted; font.pixelSize: 13
                                }
                            }
                        }
                        Item { Layout.fillHeight: true }
                        Text {
                            visible: brain.candidates.length === 0
                            text: "Press Play or Step to see the choices."
                            color: muted; font.pixelSize: 13; wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                    Text {
                        text: "Taller bar = more likely. The purple one is what it picked."
                        color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                }
            }
        }

        // ---------- controls ----------
        Rectangle {
            Layout.fillWidth: true
            height: 76; radius: 14; color: card; border.color: line
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 18; anchors.rightMargin: 18
                spacing: 18

                // play / pause
                Rectangle {
                    width: 108; height: 40; radius: 10
                    color: playMa.pressed ? "#5B43D6" : (playMa.containsMouse ? "#8B6BFF" : accent)
                    Text { anchors.centerIn: parent; color: "white"; font.pixelSize: 15; font.bold: true
                           text: brain.running ? "⏸  Pause" : "▶  Play" }
                    MouseArea { id: playMa; anchors.fill: parent; hoverEnabled: true
                        onClicked: brain.running ? brain.pause() : brain.play() }
                }
                // step
                Rectangle {
                    width: 84; height: 40; radius: 10; color: stepMa.containsMouse ? "#20263C" : "transparent"
                    border.color: line
                    Text { anchors.centerIn: parent; text: "Step"; color: "white"; font.pixelSize: 14 }
                    MouseArea { id: stepMa; anchors.fill: parent; hoverEnabled: true
                        onClicked: { brain.pause(); brain.step(); } }
                }
                // reset
                Rectangle {
                    width: 84; height: 40; radius: 10; color: resetMa.containsMouse ? "#20263C" : "transparent"
                    border.color: line
                    Text { anchors.centerIn: parent; text: "Reset"; color: muted; font.pixelSize: 14 }
                    MouseArea { id: resetMa; anchors.fill: parent; hoverEnabled: true
                        onClicked: { brain.pause(); brain.reset(); } }
                }

                Item { width: 8 }

                // temperature
                ColumnLayout {
                    spacing: 0; Layout.preferredWidth: 220
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Temperature"; color: muted; font.pixelSize: 12 }
                        Item { Layout.fillWidth: true }
                        Text { text: brain.temperature < 0.7 ? "calm" : (brain.temperature > 1.3 ? "wild" : "balanced")
                               color: "white"; font.pixelSize: 12; font.bold: true }
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: 0.2; to: 2.0; value: brain.temperature
                        onMoved: brain.temperature = value
                    }
                }

                // speed
                ColumnLayout {
                    spacing: 0; Layout.preferredWidth: 180
                    Text { text: "Speed"; color: muted; font.pixelSize: 12 }
                    Slider {
                        Layout.fillWidth: true
                        from: 0; to: 1; value: 0.45
                        onMoved: brain.setSpeed(Math.round(1300 - value * 1150))  // right = faster
                    }
                }
                Item { Layout.fillWidth: true }
            }
        }
    }
}
