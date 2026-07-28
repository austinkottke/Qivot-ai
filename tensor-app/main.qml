import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: win
    visible: true
    width: 980; height: 720
    color: "#0E1220"
    title: "Tensors — one neural layer you can play with"

    readonly property color card:   "#171B2C"
    readonly property color line:   "#262B40"
    readonly property color accent: "#22C55E"
    readonly property color muted:  "#8A8FA3"

    function bar(frac, w) { return Math.max(0, Math.min(1, frac)) * w }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 14

        // ---------- header ----------
        RowLayout {
            spacing: 12
            Rectangle { width: 34; height: 34; radius: 9; color: accent
                Text { anchors.centerIn: parent; text: "▦"; color: "white"; font.pixelSize: 18; font.bold: true } }
            ColumnLayout {
                spacing: 0
                Text { text: "Tensors — one neural layer"; color: "white"; font.pixelSize: 22; font.bold: true }
                Text { text: "Drag a colour. Watch the numbers flow through the layer — multiply, add, squash."
                       color: muted; font.pixelSize: 13 }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 16

            // =========== LEFT: the input color ===========
            Rectangle {
                Layout.fillHeight: true; Layout.preferredWidth: 2; Layout.fillWidth: true
                radius: 14; color: card; border.color: line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 12

                    Text { text: "1 · Pick a colour (the input)"; color: muted; font.pixelSize: 13 }

                    // swatch
                    Rectangle {
                        Layout.fillWidth: true; height: 74; radius: 10
                        color: Qt.rgba(net.red/255, net.green/255, net.blue/255, 1)
                        border.color: line
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }

                    // sliders (one per colour channel)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Red"; color: "#F87171"; font.pixelSize: 13; font.bold: true; Layout.preferredWidth: 46 }
                        Slider { Layout.fillWidth: true; from: 0; to: 255; value: net.red; onMoved: net.red = value }
                        Text { text: net.red; color: "white"; font.pixelSize: 13; Layout.preferredWidth: 32; horizontalAlignment: Text.AlignRight }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Green"; color: "#4ADE80"; font.pixelSize: 13; font.bold: true; Layout.preferredWidth: 46 }
                        Slider { Layout.fillWidth: true; from: 0; to: 255; value: net.green; onMoved: net.green = value }
                        Text { text: net.green; color: "white"; font.pixelSize: 13; Layout.preferredWidth: 32; horizontalAlignment: Text.AlignRight }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Blue"; color: "#60A5FA"; font.pixelSize: 13; font.bold: true; Layout.preferredWidth: 46 }
                        Slider { Layout.fillWidth: true; from: 0; to: 255; value: net.blue; onMoved: net.blue = value }
                        Text { text: net.blue; color: "white"; font.pixelSize: 13; Layout.preferredWidth: 32; horizontalAlignment: Text.AlignRight }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: line }
                    Text { text: "As a vector of numbers (scaled 0..1):"; color: muted; font.pixelSize: 12 }

                    // input bars
                    Repeater {
                        model: net.inputs
                        delegate: RowLayout {
                            Layout.fillWidth: true; spacing: 8; height: 22
                            Text { text: modelData.name; color: muted; font.pixelSize: 13; Layout.preferredWidth: 46 }
                            Rectangle {
                                Layout.fillWidth: true; height: 14; radius: 7; color: "#10152A"; border.color: line
                                Rectangle { height: parent.height; radius: 7; color: "#3A4160"
                                    width: parent.width * Math.max(0, Math.min(1, modelData.norm))
                                    Behavior on width { NumberAnimation { duration: 120 } } }
                            }
                            Text { text: modelData.norm.toFixed(2); color: "white"; font.pixelSize: 12
                                   Layout.preferredWidth: 34; horizontalAlignment: Text.AlignRight }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            // =========== RIGHT: the math ===========
            Rectangle {
                Layout.fillHeight: true; Layout.preferredWidth: 3; Layout.fillWidth: true
                radius: 14; color: card; border.color: line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 12

                    // weight grid
                    Text { text: "2 · The weight grid — a 2×3 tensor (stored in SQLite)"; color: muted; font.pixelSize: 13 }
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 6
                        RowLayout {   // column headers
                            Layout.fillWidth: true; spacing: 10
                            Item { Layout.preferredWidth: 90 }
                            Text { text: "red";   color: "#F87171"; font.pixelSize: 12; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                            Text { text: "green"; color: "#4ADE80"; font.pixelSize: 12; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                            Text { text: "blue";  color: "#60A5FA"; font.pixelSize: 12; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                        }
                        Repeater {
                            model: net.outputs
                            delegate: RowLayout {
                                Layout.fillWidth: true; spacing: 10
                                Text { text: modelData.name; color: "white"; font.pixelSize: 13; Layout.preferredWidth: 90 }
                                Repeater {
                                    model: modelData.w
                                    delegate: Rectangle {
                                        Layout.fillWidth: true; height: 30; radius: 7
                                        color: modelData >= 0 ? "#12351F" : "#3A1620"
                                        border.color: line
                                        Text { anchors.centerIn: parent
                                               text: (modelData >= 0 ? "+" : "") + modelData.toFixed(2)
                                               color: modelData >= 0 ? "#7CE0A0" : "#F19AA6"; font.pixelSize: 13 }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: line }

                    // the computation
                    Text { text: "3 · Multiply, add, squash → the output numbers"; color: muted; font.pixelSize: 13 }
                    Repeater {
                        model: net.outputs
                        delegate: ColumnLayout {
                            Layout.fillWidth: true; spacing: 4
                            Text { text: modelData.name; color: "white"; font.pixelSize: 14; font.bold: true }
                            Text { text: modelData.expr + "  =  " + modelData.sum.toFixed(2)
                                   color: muted; font.pixelSize: 12; font.family: "Menlo, monospace"
                                   Layout.fillWidth: true; wrapMode: Text.WordWrap }
                            RowLayout {
                                Layout.fillWidth: true; spacing: 8
                                Rectangle {
                                    Layout.fillWidth: true; height: 22; radius: 11; color: "#10152A"; border.color: line
                                    Rectangle { height: parent.height; radius: 11; color: accent
                                        width: parent.width * Math.max(0, Math.min(1, modelData.value))
                                        Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } } }
                                    Text { anchors { right: parent.right; rightMargin: 10; verticalCenter: parent.verticalCenter }
                                           text: "squashed to " + modelData.value.toFixed(2); color: "white"; font.pixelSize: 12 }
                                }
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                    Text {
                        text: "This whole thing is ONE layer. Real networks stack many, with millions of weights that are learned from data — not typed in like these six."
                        color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
