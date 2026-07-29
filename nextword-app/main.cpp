/** Guess the Next Word — the interactive (QML) version.

    Press Play and watch a tiny language model write, one word at a time. A side
    panel shows the words it's choosing between at each step (with their odds), so
    you can literally see it "think." The model is trained into an in-memory SQLite
    table on startup; every guess is a real query against that table.
 */
#include "wordmodel.h"
#include "model.h"
#include "corpus.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSqlDatabase>
#include <QTimer>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) return 1;

    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<Gram>();
    if (!connection.createTables()) return 1;

    WordModel brain;
    brain.train(QString::fromUtf8(kCorpus));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("brain", &brain);
    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()) return 1;

    if (qEnvironmentVariableIsSet("QIVOT_SHOT")) {
        for (int i = 0; i < 38; i++) brain.step();     // write some words to show
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QTimer::singleShot(1200, [win] {
            if (win) win->grabWindow().save(qEnvironmentVariable("QIVOT_SHOT"));
            QCoreApplication::quit();
        });
    }

    return app.exec();
}
