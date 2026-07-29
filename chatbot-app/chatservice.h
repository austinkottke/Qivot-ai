#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <QObject>
#include <QVariantList>
#include <QProcess>
#include <QTimer>
#include <QNetworkAccessManager>

/// The backend QML talks to. It:
///   1. starts a local llama.cpp server (a real open model, running on your machine),
///   2. sends your messages to it over http://localhost and reads the reply,
///   3. keeps the whole conversation in SQLite (through Qivot) so it survives restarts.
///
/// No cloud, no API key — the model file lives on disk and answers locally.
class ChatService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(bool    ready  READ ready  NOTIFY readyChanged)   ///< the model server is up
    Q_PROPERTY(bool    busy   READ busy   NOTIFY busyChanged)    ///< waiting on a reply
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit ChatService(QObject *parent = nullptr);
    ~ChatService() override;

    QVariantList messages() const { return m_messages; }
    bool    ready() const  { return m_ready; }
    bool    busy() const   { return m_busy; }
    QString status() const { return m_status; }

    Q_INVOKABLE void send(const QString &text);
    Q_INVOKABLE void clearChat();

signals:
    void messagesChanged();
    void readyChanged();
    void busyChanged();
    void statusChanged();

private:
    void loadHistory();
    void startServer();
    void checkHealth();
    void addMessage(int role, const QString &text, bool persist);
    void setStatus(const QString &s);
    void setReady(bool r);
    void setBusy(bool b);

    QString serverBinaryPath() const;
    QString modelPath() const;

    QNetworkAccessManager m_net;
    QProcess    *m_server = nullptr;
    QTimer       m_health;
    QVariantList m_messages;
    bool         m_ready = false;
    bool         m_busy = false;
    QString      m_status = "Starting the model…";
    int          m_healthTries = 0;

    static constexpr int kPort = 8080;
};

#endif // CHATSERVICE_H
