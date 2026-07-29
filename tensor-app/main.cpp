/** Tensors — the interactive (QML) version.

    Drag the red / green / blue sliders and watch one neural-network layer compute in
    real time: the input numbers, the weight grid (read from SQLite), the multiply-and-
    add, and the two output numbers all update live. It's the terminal `tensor` lesson
    turned into something you can play with.
 */
#include "layer.h"
#include "models.h"

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
    connection.addModel<Weight>();
    connection.addModel<Bias>();
    if (!connection.createTables()) return 1;

    // The knowledge: a 2x3 weight grid + 2 biases (hand-picked so it's readable).
    //            R      G      B
    //  bright  0.30   0.59   0.11
    //  warm    0.90   0.00  -0.90
    const double W[2][3] = { { 0.30, 0.59, 0.11 }, { 0.90, 0.00, -0.90 } };
    {
        QiTransaction tx;
        for (int o = 0; o < 2; o++) {
            for (int i = 0; i < 3; i++) { Weight w; w.outIdx = o; w.inIdx = i; w.value = W[o][i]; w.save(); }
            Bias b; b.idx = o; b.value = 0.0; b.save();
        }
        tx.commit();
    }

    Layer layer;
    layer.load();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("net", &layer);
    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()) return 1;

    // Set QIVOT_SHOT=path to auto-save a screenshot of the window and quit.
    if (qEnvironmentVariableIsSet("QIVOT_SHOT")) {
        layer.setRed(240); layer.setGreen(150); layer.setBlue(60);   // a warm orange
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QTimer::singleShot(1200, [win] {
            if (win) win->grabWindow().save(qEnvironmentVariable("QIVOT_SHOT"));
            QCoreApplication::quit();
        });
    }

    return app.exec();
}
