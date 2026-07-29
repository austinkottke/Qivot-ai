/** Sort into Buckets — the interactive (QML) version.

    Type a message and watch a junk-mail detector make up its mind in real time: a
    verdict badge, a normal↔junk meter, and a per-word "evidence" chart showing which
    words pushed which way. It learned all of this by counting labelled examples.
 */
#include "classifier.h"
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
    connection.addModel<WordCount>();
    if (!connection.createTables()) return 1;

    Classifier clf;
    clf.train();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("clf", &clf);
    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()) return 1;

    return app.exec();
}
