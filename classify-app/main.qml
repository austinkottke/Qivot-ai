import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: win
    visible: true
    width: 1000; height: 700
    color: "#0E1220"
    title: "Sort into Buckets — junk or normal?"

    readonly property color card:   "#171B2C"
    readonly property color line:   "#262B40"
    readonly property color junkC:  "#EF6A6A"
    readonly property color normC:  "#3FB37F"
    readonly property color muted:  "#8A8FA3"

    // A diverging bar: fills right (junk) for lean>0, left (normal) for lean<0.
    component DivBar: Item {
        property real lean: 0
        Rectangle { anchors.fill: parent; radius: 6; color: "#0E1524"; border.color: win.line }
        Rectangle { width: 2; height: parent.height; x: parent.width/2 - 1; color: "#3A4160" }
        Rectangle {
            height: parent.height - 4; y: 2; radius: 5
            width: Math.abs(parent.lean) * (parent.width/2 - 3)
            x: parent.lean >= 0 ? parent.width/2 + 1 : parent.width/2 - 1 - width
            color: parent.lean >= 0 ? win.junkC : win.normC
            Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 14

        // ---------- header ----------
        RowLayout {
            spacing: 12
            Rectangle { width: 34; height: 34; radius: 9; color: "#7C5CFF"
                Text { anchors.centerIn: parent; text: "🗂"; font.pixelSize: 17 } }
            ColumnLayout {
                spacing: 0
                Text { text: "Sort into Buckets"; color: "white"; font.pixelSize: 22; font.bold: true }
                Text { text: "A junk-mail detector that learned from " + clf.junkExamples + " junk + "
                             + clf.normalExamples + " normal messages (" + clf.vocabulary + " words)."
                       color: muted; font.pixelSize: 13 }
            }
        }

        // ---------- message box ----------
        Rectangle {
            Layout.fillWidth: true
            height: 52; radius: 13; color: card; border.color: field.activeFocus ? "#7C5CFF" : line
            Behavior on border.color { ColorAnimation { duration: 150 } }
            TextField {
                id: field
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 12
                verticalAlignment: TextInput.AlignVCenter
                placeholderText: "Type a message…  e.g.  claim your free cash prize  —  or  lunch on friday"
                placeholderTextColor: "#5A6578"
                color: "white"; font.pixelSize: 16; background: null
                onTextChanged: clf.message = text
                Component.onCompleted: { text = "claim your free cash prize now"; forceActiveFocus() }
            }
        }

        // ---------- verdict + meter ----------
        Rectangle {
            Layout.fillWidth: true; height: 96; radius: 14; color: card; border.color: line
            RowLayout {
                anchors.fill: parent; anchors.margins: 18; spacing: 18
                // badge
                Rectangle {
                    Layout.preferredWidth: 160; Layout.fillHeight: true; radius: 12
                    color: !clf.hasWords ? "#20263C" : (clf.verdict === "JUNK" ? "#3A1620" : "#12351F")
                    border.color: !clf.hasWords ? line : (clf.verdict === "JUNK" ? junkC : normC)
                    Behavior on color { ColorAnimation { duration: 150 } }
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 2
                        Text { Layout.alignment: Qt.AlignHCenter
                               text: clf.hasWords ? clf.verdict : "—"
                               color: !clf.hasWords ? muted : (clf.verdict === "JUNK" ? junkC : normC)
                               font.pixelSize: 26; font.bold: true }
                        Text { Layout.alignment: Qt.AlignHCenter
                               text: clf.hasWords ? (clf.confidence + "% sure") : "type a message"
                               color: muted; font.pixelSize: 12 }
                    }
                }
                // meter
                ColumnLayout {
                    Layout.fillWidth: true; Layout.fillHeight: true; spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "◄ normal"; color: normC; font.pixelSize: 12; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Text { text: "junk ►"; color: junkC; font.pixelSize: 12; font.bold: true }
                    }
                    DivBar { Layout.fillWidth: true; Layout.preferredHeight: 26; lean: (clf.pJunk - 0.5) * 2 }
                    Text { text: "The needle is how far the total evidence tips one way."
                           color: muted; font.pixelSize: 11 }
                }
            }
        }

        // ---------- evidence + learned ----------
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 16

            // evidence for the current message
            Rectangle {
                Layout.fillHeight: true; Layout.preferredWidth: 3; Layout.fillWidth: true
                radius: 14; color: card; border.color: line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 9
                    Text { text: "The evidence, word by word"; color: muted; font.pixelSize: 13 }
                    Repeater {
                        model: clf.evidence
                        delegate: RowLayout {
                            Layout.fillWidth: true; height: 24; spacing: 10
                            Text { text: modelData.word; color: "white"; font.pixelSize: 14
                                   Layout.preferredWidth: 120; elide: Text.ElideRight }
                            DivBar { Layout.fillWidth: true; Layout.preferredHeight: 20; lean: modelData.lean }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    Text {
                        visible: !clf.hasWords
                        text: "Type something above to see how each word votes."
                        color: muted; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                }
            }

            // what it learned
            Rectangle {
                Layout.fillHeight: true; Layout.preferredWidth: 2; Layout.fillWidth: true
                radius: 14; color: card; border.color: line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 8
                    Text { text: "What it learned"; color: "white"; font.pixelSize: 15; font.bold: true }
                    Text { text: "The strongest words each way, from the examples:"
                           color: muted; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Repeater {
                        model: clf.learned
                        delegate: RowLayout {
                            Layout.fillWidth: true; height: 22; spacing: 10
                            Text { text: modelData.word; color: modelData.lean >= 0 ? junkC : normC
                                   font.pixelSize: 13; Layout.preferredWidth: 90; elide: Text.ElideRight }
                            DivBar { Layout.fillWidth: true; Layout.preferredHeight: 16; lean: modelData.lean }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }

        Text {
            text: "It counts words, ignoring order — so \"not free\" looks like \"free\". The big AIs add meaning and context, but this is how a lot of real sorting still works."
            color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
        }
    }
}
