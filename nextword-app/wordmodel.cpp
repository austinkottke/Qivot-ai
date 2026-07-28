#include "wordmodel.h"
#include "model.h"

#include <qivot.h>
#include <QRandomGenerator>
#include <QHash>
#include <QSet>
#include <QVector>
#include <cmath>

// Break raw text into lowercase word-tokens; sentence-ending marks become a "." token.
static QStringList tokenize(const QString &text) {
    QStringList tokens;
    QString word;
    auto flush = [&] { if (!word.isEmpty()) { tokens << word; word.clear(); } };
    for (const QChar &ch : text) {
        if (ch.isLetter() || ch == '\'') word += ch.toLower();
        else { flush(); if (ch == '.' || ch == '!' || ch == '?') tokens << "."; }
    }
    flush();
    return tokens;
}

// Turn a raw context string ("." / ". the" / "the little") into a friendly label.
static QString prettyContext(const QString &ctx) {
    const QStringList parts = ctx.split(' ', Qt::SkipEmptyParts);
    QStringList real;
    for (const QString &w : parts) if (w != ".") real << w;
    if (real.isEmpty()) return QStringLiteral("the start of a sentence");
    return QLatin1Char('"') + real.join(' ') + QLatin1Char('"');
}

WordModel::WordModel(QObject *parent) : QObject(parent) {
    m_timer.setInterval(600);
    connect(&m_timer, &QTimer::timeout, this, &WordModel::step);
    for (int i = 0; i < ORDER; i++) m_recent << ".";
}

void WordModel::train(const QString &corpus) {
    const QStringList tokens = tokenize(corpus);

    QStringList stream;
    for (int i = 0; i < ORDER; i++) stream << ".";
    stream << tokens;

    QHash<QString, QHash<QString, int>> counts;
    QSet<QString> vocab;
    for (int i = ORDER; i < stream.size(); i++) {
        const QString next = stream.at(i);
        if (next != ".") vocab.insert(next);
        for (int k = 1; k <= ORDER; k++)
            counts[stream.mid(i - k, k).join(' ')][next] += 1;
    }

    int rows = 0;
    {
        QiTransaction tx;
        for (auto ci = counts.constBegin(); ci != counts.constEnd(); ++ci)
            for (auto ni = ci.value().constBegin(); ni != ci.value().constEnd(); ++ni) {
                Gram g; g.context = ci.key(); g.word = ni.key(); g.n = ni.value();
                g.save(); rows++;
            }
        tx.commit();
    }

    m_transitions = rows;
    m_vocab = vocab.size();
    m_vocabList = vocab.values();
    emit statsChanged();
    reset();
}

void WordModel::reset() {
    m_text.clear();
    m_sentenceStart = true;
    m_wordCount = 0;
    m_recent.clear();
    for (int i = 0; i < ORDER; i++) m_recent << ".";
    m_candidates.clear();
    m_contextLabel = QStringLiteral("the start of a sentence");
    emit textChanged();
    emit stepped();
}

void WordModel::step() {
    // Look up the next word: try the longest context, then back off if unseen.
    QiList<Gram> cand;
    QString usedCtx;
    for (int k = ORDER; k >= 1; k--) {
        const QString ctx = m_recent.mid(m_recent.size() - k, k).join(' ');
        cand = QiQuery<Gram>().filter(QiWhere("context = ", ctx)).orderBy("n desc").all();
        if (cand.size() > 0) { usedCtx = ctx; break; }
    }

    QString next;
    QVariantList vis;
    if (cand.size() == 0) {
        next = m_vocabList.isEmpty() ? QStringLiteral(".")
                 : m_vocabList.at(QRandomGenerator::global()->bounded(m_vocabList.size()));
        m_contextLabel = QStringLiteral("(no match — picked a random word)");
    } else {
        double total = 0.0;
        QVector<double> w(cand.size());
        for (int j = 0; j < cand.size(); j++) {
            const double c = int(cand.at(j)->n);
            w[j] = std::pow(c, 1.0 / qMax(0.01, m_temperature));
            total += w[j];
        }
        double r = QRandomGenerator::global()->generateDouble() * total;
        int pick = cand.size() - 1;
        for (int j = 0; j < cand.size(); j++) { r -= w[j]; if (r <= 0.0) { pick = j; break; } }
        next = cand.at(pick)->word;

        for (int j = 0; j < cand.size() && j < 8; j++) {
            const QString wd = cand.at(j)->word;
            QVariantMap m;
            m["word"]   = (wd == ".") ? QStringLiteral("[ end . ]") : wd;
            m["prob"]   = total > 0 ? w[j] / total : 0.0;
            m["count"]  = int(cand.at(j)->n);
            m["chosen"] = (j == pick);
            vis << m;
        }
        m_contextLabel = prettyContext(usedCtx);
    }

    m_candidates = vis;

    if (next == ".") {
        m_text = m_text.trimmed() + ". ";
        m_sentenceStart = true;
        m_recent.clear();
        for (int i = 0; i < ORDER; i++) m_recent << ".";
    } else {
        appendWord(next);
        m_recent << next;
        while (m_recent.size() > ORDER) m_recent.removeFirst();
        m_wordCount++;
    }

    emit stepped();
    emit textChanged();

    if (m_wordCount >= 80 && m_timer.isActive()) pause();   // soft stop so it doesn't run forever
}

void WordModel::appendWord(const QString &w) {
    QString s = w;
    if (m_sentenceStart && !s.isEmpty()) { s[0] = s[0].toUpper(); m_sentenceStart = false; }
    m_text += s + " ";
}

void WordModel::setTemperature(double t) {
    if (qFuzzyCompare(t, m_temperature)) return;
    m_temperature = t;
    emit temperatureChanged();
}

void WordModel::play()  { if (!m_timer.isActive()) { m_timer.start(); emit runningChanged(); } }
void WordModel::pause() { if (m_timer.isActive())  { m_timer.stop();  emit runningChanged(); } }
void WordModel::setSpeed(int ms) { m_timer.setInterval(qBound(120, ms, 2000)); }
