#ifndef WORDMODEL_H
#define WORDMODEL_H

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QTimer>

/// The backend QML talks to. It trains a tiny "guess the next word" model into a
/// SQLite table, then produces text one word at a time — exposing, at each step,
/// the candidate words it's weighing (so the UI can show it "thinking").
class WordModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString      text        READ text        NOTIFY textChanged)
    Q_PROPERTY(QString      context     READ context     NOTIFY stepped)
    Q_PROPERTY(QVariantList candidates  READ candidates  NOTIFY stepped)
    Q_PROPERTY(double       temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(bool         running     READ running     NOTIFY runningChanged)
    Q_PROPERTY(int          transitions READ transitions NOTIFY statsChanged)
    Q_PROPERTY(int          vocabulary  READ vocabulary  NOTIFY statsChanged)

public:
    explicit WordModel(QObject *parent = nullptr);

    /// Read the text, count word patterns, and save them into the database.
    void train(const QString &corpus);

    QString      text() const        { return m_text; }
    QString      context() const     { return m_contextLabel; }
    QVariantList candidates() const  { return m_candidates; }
    double       temperature() const { return m_temperature; }
    bool         running() const     { return m_timer.isActive(); }
    int          transitions() const { return m_transitions; }
    int          vocabulary() const  { return m_vocab; }

    void setTemperature(double t);

    Q_INVOKABLE void reset();          ///< clear the writing, back to a sentence start
    Q_INVOKABLE void step();           ///< produce exactly one more word
    Q_INVOKABLE void play();           ///< auto-step on a timer
    Q_INVOKABLE void pause();
    Q_INVOKABLE void setSpeed(int ms); ///< delay between words when playing

signals:
    void textChanged();
    void stepped();
    void temperatureChanged();
    void runningChanged();
    void statsChanged();

private:
    void appendWord(const QString &w);

    static const int ORDER = 2;        // look back this many words to guess the next

    QTimer       m_timer;
    QStringList  m_recent;             // the last few words written
    QString      m_text;
    QString      m_contextLabel;
    QVariantList m_candidates;
    double       m_temperature = 1.0;
    bool         m_sentenceStart = true;
    int          m_wordCount = 0;
    int          m_transitions = 0;
    int          m_vocab = 0;
    QStringList  m_vocabList;
};

#endif // WORDMODEL_H
