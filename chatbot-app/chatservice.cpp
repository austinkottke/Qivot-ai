#include "chatservice.h"
#include "message.h"

#include <qivot.h>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

ChatService::ChatService(QObject *parent) : QObject(parent) {
    loadHistory();

    connect(&m_health, &QTimer::timeout, this, &ChatService::checkHealth);
    m_health.setInterval(700);

    startServer();
    m_health.start();
    checkHealth();          // probe immediately (in case a server is already running)
}

ChatService::~ChatService() {
    if (m_server) {
        m_server->terminate();
        if (!m_server->waitForFinished(1500)) m_server->kill();
    }
}

QString ChatService::serverBinaryPath() const {
    const QString env = qEnvironmentVariable("LLAMA_SERVER");
    if (!env.isEmpty()) return env;
    const QString app = QCoreApplication::applicationDirPath();
    return QDir::cleanPath(app + "/../../llama.cpp/build/bin/llama-server");
}

QString ChatService::modelPath() const {
    const QString env = qEnvironmentVariable("LLAMA_MODEL");
    if (!env.isEmpty()) return env;
    const QString app = QCoreApplication::applicationDirPath();
    return QDir::cleanPath(app + "/models/qwen2.5-0.5b-instruct-q4_k_m.gguf");
}

void ChatService::startServer() {
    const QString bin = serverBinaryPath();
    const QString model = modelPath();

    if (!QFileInfo::exists(bin)) {
        setStatus("llama-server not found.\nBuild llama.cpp, or set LLAMA_SERVER to its path.");
        return;
    }
    if (!QFileInfo::exists(model)) {
        setStatus("Model file not found at " + model);
        return;
    }

    m_server = new QProcess(this);
    m_server->setProcessChannelMode(QProcess::MergedChannels);
    // -ngl 99 offloads all layers to the GPU (Metal on Apple Silicon) = fast.
    m_server->start(bin, { "-m", model,
                           "--host", "127.0.0.1",
                           "--port", QString::number(kPort),
                           "-c", "4096",
                           "-ngl", "99" });
    setStatus("Starting the model…");
}

void ChatService::checkHealth() {
    if (m_ready) return;
    QNetworkRequest req(QUrl(QString("http://127.0.0.1:%1/health").arg(kPort)));
    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError && body.contains("ok");
        reply->deleteLater();
        if (ok) {
            m_health.stop();
            setReady(true);
            setStatus(m_messages.isEmpty() ? "Ready — say hi 👋" : "Ready");
        } else if (++m_healthTries > 90) {          // ~60s
            m_health.stop();
            setStatus("The model server didn't come up. Check the terminal it launched in.");
        }
    });
}

void ChatService::send(const QString &text) {
    const QString t = text.trimmed();
    if (t.isEmpty() || !m_ready || m_busy) return;

    addMessage(0, t, true);
    setBusy(true);
    setStatus("Thinking…");

    // Build the conversation for the model (a system nudge + the whole history).
    QJsonArray msgs;
    {
        QJsonObject sys;
        sys["role"] = "system";
        sys["content"] = "You are a helpful, friendly assistant running locally on the "
                         "user's computer. Keep answers clear and fairly concise.";
        msgs.append(sys);
    }
    for (const QVariant &v : m_messages) {
        const QVariantMap m = v.toMap();
        QJsonObject o;
        o["role"] = m["role"].toInt() == 0 ? "user" : "assistant";
        o["content"] = m["text"].toString();
        msgs.append(o);
    }

    QJsonObject body;
    body["messages"] = msgs;
    body["stream"] = false;
    body["temperature"] = 0.7;
    body["max_tokens"] = 512;

    QNetworkRequest req(QUrl(QString("http://127.0.0.1:%1/v1/chat/completions").arg(kPort)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_net.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        QString answer;
        if (reply->error() == QNetworkReply::NoError) {
            const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
            const QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty())
                answer = choices.first().toObject()["message"].toObject()["content"].toString().trimmed();
        }
        reply->deleteLater();
        if (answer.isEmpty()) answer = "⚠️ (no reply — is the model still loading?)";
        addMessage(1, answer, true);
        setBusy(false);
        setStatus("Ready");
    });
}

void ChatService::clearChat() {
    QiQuery<Message>().filter(QiWhere("id > ", 0)).remove();
    m_messages.clear();
    emit messagesChanged();
    setStatus(m_ready ? "Ready — say hi 👋" : m_status);
}

void ChatService::addMessage(int role, const QString &text, bool persist) {
    QVariantMap m;
    m["role"] = role;
    m["text"] = text;
    m_messages.append(m);
    emit messagesChanged();

    if (persist) {
        Message row;
        row.role = role;
        row.text = text;
        row.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
        row.save();
    }
}

void ChatService::loadHistory() {
    QiList<Message> rows = QiQuery<Message>().orderBy("id asc").all();
    for (int i = 0; i < rows.size(); i++) {
        QVariantMap m;
        m["role"] = int(rows.at(i)->role);
        m["text"] = QString(rows.at(i)->text);
        m_messages.append(m);
    }
    if (!m_messages.isEmpty()) emit messagesChanged();
}

void ChatService::setStatus(const QString &s) { if (s != m_status) { m_status = s; emit statusChanged(); } }
void ChatService::setReady(bool r)  { if (r != m_ready) { m_ready = r; emit readyChanged(); } }
void ChatService::setBusy(bool b)   { if (b != m_busy)  { m_busy = b; emit busyChanged(); } }
