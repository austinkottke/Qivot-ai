#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QVector>
#include <QHash>

/// The backend QML talks to. It learns word counts from the labelled examples
/// (saving them into SQLite), then for whatever message you type it scores the two
/// buckets and exposes the per-word evidence so the UI can draw it live.
class Classifier : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString      message  READ message WRITE setMessage NOTIFY changed)
    Q_PROPERTY(QVariantList evidence READ evidence NOTIFY changed)   ///< per word: {word, lean}
    Q_PROPERTY(double       pJunk    READ pJunk    NOTIFY changed)   ///< 0 = normal … 1 = junk
    Q_PROPERTY(QString      verdict  READ verdict  NOTIFY changed)
    Q_PROPERTY(int          confidence READ confidence NOTIFY changed)
    Q_PROPERTY(bool         hasWords READ hasWords NOTIFY changed)
    Q_PROPERTY(QVariantList learned  READ learned  NOTIFY statsChanged) ///< strongest words each way
    Q_PROPERTY(int junkExamples   READ junkExamples   NOTIFY statsChanged)
    Q_PROPERTY(int normalExamples READ normalExamples NOTIFY statsChanged)
    Q_PROPERTY(int vocabulary     READ vocabulary     NOTIFY statsChanged)

public:
    explicit Classifier(QObject *parent = nullptr);

    void train();                        ///< learn from the examples, save into SQLite

    QString      message() const  { return m_message; }
    QVariantList evidence() const { return m_evidence; }
    double       pJunk() const    { return m_pJunk; }
    QString      verdict() const  { return m_verdict; }
    int          confidence() const { return m_confidence; }
    bool         hasWords() const { return m_hasWords; }
    QVariantList learned() const  { return m_learned; }
    int junkExamples() const;
    int normalExamples() const;
    int vocabulary() const { return m_V; }

    void setMessage(const QString &m);

signals:
    void changed();
    void statsChanged();

private:
    void   recompute();
    double logP(int cls, int wordCount) const;
    double leanOf(const QString &w) const;   ///< clamped to [-1, 1] for the UI

    QString      m_message;
    QVariantList m_evidence;
    QVariantList m_learned;
    double       m_pJunk = 0.5;
    QString      m_verdict = "—";
    int          m_confidence = 0;
    bool         m_hasWords = false;

    QHash<QString, QVector<int>> m_cnt;      // m_cnt[word] = { normal, junk }
    int m_totalWords[2] = { 0, 0 };
    int m_V = 0;
};

#endif // CLASSIFIER_H
