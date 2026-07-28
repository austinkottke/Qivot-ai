#include "searchmodel.h"
#include "models.h"
#include "corpus.h"

#include <qivot.h>
#include <QSet>
#include <cmath>
#include <algorithm>

static const QSet<QString> kStop = {
    "the","a","an","and","or","of","to","in","on","at","is","are","was","were",
    "it","its","this","that","for","with","as","by","be","i","you","my","your",
    "how","do","does","can","what","when","which","me","we","they","them","their",
    "from","into","out","up","down","so","if","then","also","until","through"
};

static QStringList topicWords(const QString &text) {
    QStringList res; QString w;
    auto flush = [&] { if (!w.isEmpty()) { if (!kStop.contains(w)) res << w; w.clear(); } };
    for (const QChar &c : text) { if (c.isLetter()) w += c.toLower(); else flush(); }
    flush();
    return res;
}

SearchModel::SearchModel(QObject *parent) : QObject(parent) {}

double SearchModel::idf(const QString &w) const {
    return std::log(double(m_N) / (1.0 + m_docFreq.value(w, 0))) + 1.0;
}

void SearchModel::build() {
    m_N = kDocCount;
    QVector<QStringList> docWords(m_N);
    for (int i = 0; i < m_N; i++) {
        docWords[i] = topicWords(QString::fromUtf8(kDocs[i].text));
        QSet<QString> uniq(docWords[i].begin(), docWords[i].end());
        for (const QString &w : uniq) m_docFreq[w] += 1;
    }

    m_docId.resize(m_N); m_docNorm.resize(m_N);
    QiTransaction tx;
    for (int i = 0; i < m_N; i++) {
        QHash<QString,int> tf;
        for (const QString &w : docWords[i]) tf[w] += 1;

        double sumSq = 0.0;
        QHash<QString,double> weight;
        for (auto it = tf.constBegin(); it != tf.constEnd(); ++it) {
            const double wt = it.value() * idf(it.key());
            weight[it.key()] = wt; sumSq += wt * wt;
        }
        const double norm = std::sqrt(sumSq);

        Doc d; d.title = kDocs[i].title; d.text = kDocs[i].text; d.norm = norm;
        d.save();
        const int id = d.id.get().toInt();
        m_docId[i] = id; m_docNorm[i] = norm;
        m_titles << kDocs[i].title; m_texts << QString::fromUtf8(kDocs[i].text);

        for (auto it = weight.constBegin(); it != weight.constEnd(); ++it) {
            Term t; t.docId = id; t.word = it.key(); t.weight = it.value(); t.save();
        }
    }
    tx.commit();

    recompute();
}

void SearchModel::setQuery(const QString &q) {
    if (q == m_query) return;
    m_query = q;
    emit queryChanged();
    recompute();
}

void SearchModel::recompute() {
    m_results.clear(); m_bestWords.clear(); m_keywords.clear();
    m_bestTitle.clear(); m_bestText.clear();

    const QStringList qWords = topicWords(m_query);
    QHash<QString,int> qtf;
    for (const QString &w : qWords) qtf[w] += 1;
    for (auto it = qtf.constBegin(); it != qtf.constEnd(); ++it) m_keywords << it.key();

    QHash<QString,double> qWeight; double qSumSq = 0.0;
    for (auto it = qtf.constBegin(); it != qtf.constEnd(); ++it) {
        const double wt = it.value() * idf(it.key());
        qWeight[it.key()] = wt; qSumSq += wt * wt;
    }
    const double qNorm = std::sqrt(qSumSq);

    // Inverted-index lookup: for each query word, which notes use it (and how much)?
    QHash<int,double> dot;
    if (qNorm > 0) {
        for (auto it = qWeight.constBegin(); it != qWeight.constEnd(); ++it) {
            QiList<Term> postings = QiQuery<Term>().filter(QiWhere("word = ", it.key())).all();
            for (int k = 0; k < postings.size(); k++)
                dot[int(postings.at(k)->docId)] += it.value() * double(postings.at(k)->weight);
        }
    }

    struct R { int i; double sim; };
    QVector<R> ranked;
    for (int i = 0; i < m_N; i++) {
        const double d = dot.value(m_docId[i], 0.0);
        const double sim = (qNorm > 0 && m_docNorm[i] > 0) ? d / (qNorm * m_docNorm[i]) : 0.0;
        ranked.push_back({ i, sim });
    }
    std::sort(ranked.begin(), ranked.end(), [](const R &a, const R &b) { return a.sim > b.sim; });

    const double best = (ranked.isEmpty() || ranked[0].sim <= 0) ? 1.0 : ranked[0].sim;
    for (const R &r : ranked) {
        QVariantMap m;
        m["title"] = m_titles[r.i];
        m["score"] = r.sim;
        m["frac"]  = r.sim / best;
        m["hit"]   = r.sim > 0;
        m_results << m;
    }

    if (!ranked.isEmpty() && ranked[0].sim > 0) {
        const int bi = ranked[0].i;
        m_bestTitle = m_titles[bi];
        m_bestText  = m_texts[bi];
        QSet<QString> qset;
        for (const QString &w : m_keywords) qset.insert(w);

        QiList<Term> tl = QiQuery<Term>().filter(QiWhere("docId = ", m_docId[bi]))
                                         .orderBy("weight desc").all();
        const double top = tl.size() > 0 ? double(tl.at(0)->weight) : 1.0;
        for (int j = 0; j < tl.size() && j < 7; j++) {
            QVariantMap m;
            const QString wd = tl.at(j)->word;
            m["word"]    = wd;
            m["frac"]    = double(tl.at(j)->weight) / top;
            m["inQuery"] = qset.contains(wd);
            m_bestWords << m;
        }
    }

    emit resultsChanged();
}
