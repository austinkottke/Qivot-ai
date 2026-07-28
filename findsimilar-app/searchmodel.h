#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QVector>
#include <QHash>

/// The backend QML talks to. It builds a little search index in SQLite, then for
/// whatever you type it turns the words into numbers, compares against every note,
/// and hands QML a ranked list (plus the winner's fingerprint) to draw.
class SearchModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString      query     READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QVariantList results   READ results   NOTIFY resultsChanged)
    Q_PROPERTY(QStringList  keywords  READ keywords  NOTIFY resultsChanged)
    Q_PROPERTY(QString      bestTitle READ bestTitle NOTIFY resultsChanged)
    Q_PROPERTY(QString      bestText  READ bestText  NOTIFY resultsChanged)
    Q_PROPERTY(QVariantList bestWords READ bestWords NOTIFY resultsChanged)
    Q_PROPERTY(int          docCount  READ docCount  CONSTANT)

public:
    explicit SearchModel(QObject *parent = nullptr);

    void build();                        ///< read the notes, save their fingerprints

    QString      query() const     { return m_query; }
    QVariantList results() const   { return m_results; }
    QStringList  keywords() const  { return m_keywords; }
    QString      bestTitle() const { return m_bestTitle; }
    QString      bestText() const  { return m_bestText; }
    QVariantList bestWords() const { return m_bestWords; }
    int          docCount() const  { return m_N; }

    void setQuery(const QString &q);

signals:
    void queryChanged();
    void resultsChanged();

private:
    void   recompute();
    double idf(const QString &w) const;

    QString      m_query;
    QVariantList m_results;
    QVariantList m_bestWords;
    QStringList  m_keywords;
    QString      m_bestTitle;
    QString      m_bestText;

    int              m_N = 0;
    QVector<int>     m_docId;
    QVector<double>  m_docNorm;
    QStringList      m_titles;
    QStringList      m_texts;
    QHash<QString,int> m_docFreq;
};

#endif // SEARCHMODEL_H
