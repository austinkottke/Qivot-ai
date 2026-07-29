#include "classifier.h"
#include "models.h"
#include "corpus.h"

#include <qivot.h>
#include <QSet>
#include <cmath>
#include <algorithm>

static const QSet<QString> kStop = {
    "a","an","the","to","for","of","in","on","at","is","are","be","you","your",
    "i","we","me","my","this","that","with","and","or","it","so","have","has"
};

static QStringList words(const QString &text) {
    QStringList res; QString w;
    auto flush = [&] { if (!w.isEmpty()) { if (!kStop.contains(w)) res << w; w.clear(); } };
    for (const QChar &c : text) { if (c.isLetter()) w += c.toLower(); else flush(); }
    flush();
    return res;
}

Classifier::Classifier(QObject *parent) : QObject(parent) {}

int Classifier::junkExamples() const   { return kJunkCount; }
int Classifier::normalExamples() const { return kNormalCount; }

double Classifier::logP(int cls, int wordCount) const {
    return std::log((wordCount + 1.0) / (m_totalWords[cls] + m_V));
}

double Classifier::leanOf(const QString &w) const {
    const QVector<int> c = m_cnt.value(w, QVector<int>{ 0, 0 });
    const double l = logP(1, c[1]) - logP(0, c[0]);
    return std::max(-1.0, std::min(1.0, l / 2.5));       // scaled for the UI
}

void Classifier::train() {
    QSet<QString> vocab;
    auto learn = [&](const char *msg, int cls) {
        for (const QString &w : words(QString::fromUtf8(msg))) {
            if (!m_cnt.contains(w)) m_cnt[w] = QVector<int>{ 0, 0 };
            m_cnt[w][cls]++; m_totalWords[cls]++; vocab.insert(w);
        }
    };
    for (int i = 0; i < kNormalCount; i++) learn(kNormal[i], 0);
    for (int i = 0; i < kJunkCount;   i++) learn(kJunk[i],   1);
    m_V = vocab.size();

    // Save the learned counts into SQLite (the persistent "brain").
    QiTransaction tx;
    for (auto it = m_cnt.constBegin(); it != m_cnt.constEnd(); ++it)
        for (int c = 0; c < 2; c++)
            if (it.value()[c] > 0) {
                WordCount wc; wc.word = it.key(); wc.cls = c; wc.n = it.value()[c];
                wc.save();
            }
    tx.commit();

    // The strongest-leaning words each way, for the "what it learned" panel.
    QVector<QPair<QString, double>> leans;
    for (const QString &w : vocab) {
        const QVector<int> c = m_cnt.value(w);
        if (c[0] + c[1] >= 2) leans.push_back({ w, leanOf(w) });
    }
    std::sort(leans.begin(), leans.end(), [](auto &a, auto &b) { return a.second > b.second; });
    auto add = [&](const QString &w, double l) {
        QVariantMap m; m["word"] = w; m["lean"] = l; m_learned << m;
    };
    for (int i = 0; i < leans.size() && i < 5; i++) add(leans[i].first, leans[i].second);
    for (int i = qMax(5, leans.size() - 5); i < leans.size(); i++) add(leans[i].first, leans[i].second);

    emit statsChanged();
    recompute();
}

void Classifier::setMessage(const QString &m) {
    if (m == m_message) return;
    m_message = m;
    recompute();
}

void Classifier::recompute() {
    const QStringList qw = words(m_message);
    m_hasWords = !qw.isEmpty();

    m_evidence.clear();
    double score[2] = { std::log(kNormalCount / double(kNormalCount + kJunkCount)),
                        std::log(kJunkCount   / double(kNormalCount + kJunkCount)) };
    for (const QString &w : qw) {
        const QVector<int> c = m_cnt.value(w, QVector<int>{ 0, 0 });
        score[0] += logP(0, c[0]);
        score[1] += logP(1, c[1]);
        QVariantMap m; m["word"] = w; m["lean"] = leanOf(w);
        m_evidence << m;
    }

    m_pJunk = m_hasWords ? 1.0 / (1.0 + std::exp(score[0] - score[1])) : 0.5;
    if (!m_hasWords) { m_verdict = "—"; m_confidence = 0; }
    else {
        const bool junk = m_pJunk >= 0.5;
        m_verdict = junk ? "JUNK" : "NORMAL";
        m_confidence = int(std::lround((junk ? m_pJunk : 1 - m_pJunk) * 100));
    }
    emit changed();
}
