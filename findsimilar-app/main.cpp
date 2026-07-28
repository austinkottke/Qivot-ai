/** Find Similar — the interactive (QML) version.

    Type a search and watch the notes re-sort in real time, each with a bar showing
    how close it is. A side panel shows the winning note's number-"fingerprint," with
    the words you actually searched for lit up. All matching is done by turning text
    into numbers and comparing them — no understanding, just distance.
 */
#include "searchmodel.h"
#include "models.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlDatabase>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) return 1;

    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<Doc>();
    connection.addModel<Term>();
    if (!connection.createTables()) return 1;

    SearchModel search;
    search.build();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("search", &search);
    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()) return 1;

    return app.exec();
}
