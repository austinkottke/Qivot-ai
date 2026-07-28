import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: win
    visible: true
    width: 1000; height: 680
    color: "#0B1220"
    title: "Find Similar — search by meaning"

    readonly property color card:   "#141A2C"
    readonly property color line:   "#243049"
    readonly property color accent: "#3B82F6"
    readonly property color muted:  "#8A93A6"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 14

        // ---------- header ----------
        RowLayout {
            spacing: 12
            Rectangle { width: 34; height: 34; radius: 9; color: accent
                Text { anchors.centerIn: parent; text: "🔍"; font.pixelSize: 17 } }
            ColumnLayout {
                spacing: 0
                Text { text: "Find Similar"; color: "white"; font.pixelSize: 22; font.bold: true }
                Text { text: "Type anything. It ranks " + search.docCount + " notes by how close the numbers are — no keywords needed."
                       color: muted; font.pixelSize: 13 }
            }
        }

        // ---------- search box ----------
        Rectangle {
            Layout.fillWidth: true
            height: 52; radius: 13; color: card; border.color: field.activeFocus ? accent : line
            Behavior on border.color { ColorAnimation { duration: 150 } }
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 12; spacing: 10
                Text { text: "🔍"; font.pixelSize: 16; opacity: 0.7 }
                TextField {
                    id: field
                    Layout.fillWidth: true
                    placeholderText: "Try:  teach my dog to sit   ·   stars at night   ·   a warm morning drink"
                    placeholderTextColor: "#5A6578"
                    color: "white"; font.pixelSize: 16; background: null
                    onTextChanged: search.query = text
                    Component.onCompleted: forceActiveFocus()
                }
                Text {
                    visible: search.keywords.length > 0
                    text: "keywords: " + search.keywords.join(", ")
                    color: muted; font.pixelSize: 12
                }
            }
        }

        // ---------- main area ----------
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 16

            // ----- results -----
            Rectangle {
                Layout.fillHeight: true; Layout.preferredWidth: 3; Layout.fillWidth: true
                radius: 14; color: card; border.color: line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 10
                    Text { text: "Notes, closest first"; color: muted; font.pixelSize: 13 }
                    ColumnLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 9
                        Repeater {
                            model: search.results
                            delegate: Item {
                                Layout.fillWidth: true
                                height: 34
                                Text {
                                    id: t
                                    width: 130; anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.title
                                    color: (index === 0 && modelData.hit) ? "white" : muted
                                    font.pixelSize: 14; font.bold: (index === 0 && modelData.hit)
                                    elide: Text.ElideRight
                                }
                                Rectangle {
                                    anchors { left: t.right; leftMargin: 10; right: sc.left; rightMargin: 10
                                              verticalCenter: parent.verticalCenter }
                                    height: 18; radius: 9; color: "#0E1524"; border.color: line
                                    Rectangle {
                                        height: parent.height; radius: 9
                                        width: Math.max(modelData.hit ? 6 : 0, parent.width * modelData.frac)
                                        color: (index === 0 && modelData.hit) ? accent : "#2A3550"
                                        Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                                    }
                                }
                                Text {
                                    id: sc
                                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                                    width: 44; horizontalAlignment: Text.AlignRight
                                    text: modelData.score.toFixed(2)
                                    color: (index === 0 && modelData.hit) ? "white" : muted; font.pixelSize: 13
                                }
                            }
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }

            // ----- why it matched -----
            Rectangle {
                Layout.fillHeight: true; Layout.preferredWidth: 2; Layout.fillWidth: true
                radius: 14; color: card; border.color: line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 10

                    Text { text: search.bestTitle.length ? "Why \"" + search.bestTitle + "\"?" : "Why it matched"
                           color: "white"; font.pixelSize: 16; font.bold: true
                           elide: Text.ElideRight; Layout.fillWidth: true }

                    Text {
                        visible: search.bestText.length
                        text: search.bestText; color: muted; font.pixelSize: 12
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    Rectangle { visible: search.bestWords.length; Layout.fillWidth: true; height: 1; color: line }
                    Text {
                        visible: search.bestWords.length
                        text: "Its fingerprint words (lit = you searched for it):"
                        color: muted; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 7
                        Repeater {
                            model: search.bestWords
                            delegate: Item {
                                Layout.fillWidth: true; height: 24
                                Text {
                                    id: bw; width: 80; anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.word
                                    color: modelData.inQuery ? "white" : muted
                                    font.pixelSize: 13; font.bold: modelData.inQuery
                                }
                                Rectangle {
                                    anchors { left: bw.right; leftMargin: 8; right: parent.right
                                              verticalCenter: parent.verticalCenter }
                                    height: 12; radius: 6; color: "#0E1524"
                                    Rectangle {
                                        height: parent.height; radius: 6
                                        width: Math.max(4, parent.width * modelData.frac)
                                        color: modelData.inQuery ? accent : "#2A3550"
                                    }
                                }
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    Text {
                        visible: !search.bestText.length
                        text: "Start typing above. Even words that aren't in any note are fine —\nyou'll just get low scores everywhere."
                        color: muted; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                }
            }
        }

        Text {
            text: "It matches on shared words. The big AIs use smarter numbers (\"embeddings\") that also match synonyms — same recipe, fancier fingerprints."
            color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
        }
    }
}
