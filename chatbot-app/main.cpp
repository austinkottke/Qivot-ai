/** Local Chatbot — a real open model, running on your machine, wired up with Qivot.

    This one is different from the other lessons: instead of building the "brain"
    from scratch, we DOWNLOAD a small pre-trained model (Qwen2.5-0.5B) that someone
    else already trained, and run it locally with llama.cpp. No cloud, no API key.

    The app:
      • starts a local llama.cpp server pointed at the model file,
      • sends your messages to it over http://localhost and shows the replies,
      • saves the whole conversation in SQLite (through Qivot) so it survives restarts.
 */
#include "chatservice.h"
#include "message.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSqlDatabase>
#include <QDir>
#include <QTimer>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);

    const bool shot = qEnvironmentVariableIsSet("QIVOT_SHOT");

    // Chat history lives in a real file next to the app, so it persists. (Screenshot
    // mode uses a throwaway in-memory DB so it never touches your real history.)
    const QString dbPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("chat.db");
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(shot ? ":memory:" : dbPath);
    if (!db.open()) return 1;

    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<Message>();
    connection.createTables();     // keeps existing history; just ensures the table exists

    ChatService chat;              // loads history, launches the model server

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("chat", &chat);
    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()) return 1;

    // Screenshot mode: once ready, ask one thing, then grab the window and quit.
    if (shot) {
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        bool *asked = new bool(false);
        auto ask = [&chat, asked] {
            if (chat.ready() && !*asked) { *asked = true; chat.send("Give me one fun fact about the ocean, in a single sentence."); }
        };
        QObject::connect(&chat, &ChatService::readyChanged, ask);
        QObject::connect(&chat, &ChatService::busyChanged, [win, &chat, asked] {
            if (*asked && !chat.busy())
                QTimer::singleShot(600, [win] {
                    if (win) win->grabWindow().save(qEnvironmentVariable("QIVOT_SHOT"));
                    QCoreApplication::quit();
                });
        });
        ask();
    }

    return app.exec();
}
