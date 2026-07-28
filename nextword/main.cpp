/** Guess the Next Word — a tiny "language model" built on Qivot.

    Chatbots write by guessing the next word, over and over. This program does the
    same trick in the simplest possible way:

      1. LEARN   — read some example text and count which word tends to follow which.
      2. SAVE    — store those counts as rows in a SQLite table (through Qivot).
                   That table IS the model — you can open nextword.db and read it.
      3. WRITE   — start a sentence, then keep asking the table "what word usually
                   comes next?" and appending the answer.

    No internet, no AI service — just counting and weighted dice-rolls over a database.

    Run it:
        ./nextword               # ~40 words, normal randomness
        ./nextword 60            # 60 words
        ./nextword 60 0.4        # calmer, more predictable
        ./nextword 60 1.5        # wilder, more surprising
 */
#include "model.h"
#include "corpus.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QRandomGenerator>
#include <QStringList>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QDebug>
#include <cmath>

// How many previous words we look at to guess the next one.
// 1 = "given the last word"; 2 = "given the last two words" (more coherent).
// Try changing this to 1 and rebuilding to see the writing get more random.
static const int ORDER = 2;

// Break raw text into a stream of lowercase word-tokens. A sentence-ending mark
// (. ! ?) becomes its own "." token, so the model can learn where sentences stop.
static QStringList tokenize(const QString &text) {
    QStringList tokens;
    QString word;
    auto flush = [&] { if (!word.isEmpty()) { tokens << word; word.clear(); } };
    for (const QChar &ch : text) {
        if (ch.isLetter() || ch == '\'') {
            word += ch.toLower();
        } else {
            flush();
            if (ch == '.' || ch == '!' || ch == '?')
                tokens << ".";
        }
    }
    flush();
    return tokens;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    const int    maxWords    = (argc > 1) ? QString(argv[1]).toInt()    : 40;
    const double temperature = (argc > 2) ? QString(argv[2]).toDouble() : 1.0;

    // --- open the database file that will hold the model ---
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("nextword.db");   // a real file you can inspect afterwards
    if (!db.open()) { qWarning() << "cannot open database"; return 1; }

    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<Gram>();
    if (!connection.dropTables() || !connection.createTables()) return 1;  // fresh each run

    // -----------------------------------------------------------------
    // 1) LEARN — count which word follows which (in memory first, it's fast).
    // -----------------------------------------------------------------
    QStringList tokens = tokenize(QString::fromUtf8(kCorpus));

    // Pad the front with sentence-start markers so the first real words have a
    // context to be looked up under.
    QStringList stream;
    for (int i = 0; i < ORDER; i++) stream << ".";
    stream << tokens;

    // counts[context][nextWord] = how many times we saw nextWord after context
    QHash<QString, QHash<QString, int>> counts;
    QSet<QString> vocab;

    for (int i = ORDER; i < stream.size(); i++) {
        const QString next = stream.at(i);
        if (next != ".") vocab.insert(next);
        // Record this for every context length 1..ORDER, so that later, if the
        // full 2-word context was never seen, we can "back off" to just 1 word.
        for (int k = 1; k <= ORDER; k++) {
            const QString ctx = stream.mid(i - k, k).join(" ");
            counts[ctx][next] += 1;
        }
    }

    // -----------------------------------------------------------------
    // 2) SAVE — write the model into the database, one row per context->word.
    //    All inside one transaction, so the many inserts commit together (fast).
    // -----------------------------------------------------------------
    int rows = 0;
    {
        QiTransaction tx;                                  // BEGIN
        for (auto ci = counts.constBegin(); ci != counts.constEnd(); ++ci) {
            const QHash<QString, int> &nexts = ci.value();
            for (auto ni = nexts.constBegin(); ni != nexts.constEnd(); ++ni) {
                Gram g;
                g.context = ci.key();
                g.word    = ni.key();
                g.n       = ni.value();
                g.save();
                rows++;
            }
        }
        tx.commit();                                       // COMMIT
    }

    qInfo().noquote() << QString("Learned %1 transitions from %2 words (vocabulary: %3 words).")
                             .arg(rows).arg(tokens.size()).arg(vocab.size());

    // -----------------------------------------------------------------
    // 3) WRITE — keep asking the database for the next word.
    // -----------------------------------------------------------------
    const QStringList vocabList = vocab.values();

    // Random normally; fixed and repeatable under QIVOT_SELFTEST so tests are stable.
    QRandomGenerator rng = qEnvironmentVariableIsSet("QIVOT_SELFTEST")
                             ? QRandomGenerator(1234u)
                             : QRandomGenerator::securelySeeded();

    QStringList recent;                          // the last few words we wrote
    for (int i = 0; i < ORDER; i++) recent << ".";   // pretend we're at a sentence start

    QStringList out;
    int wordsWritten = 0;
    while (wordsWritten < maxWords) {
        // Ask the DB: given the recent words, what usually comes next? Try the
        // longest context first, then shorten it ("back off") if we've never seen it.
        QiList<Gram> cand;
        for (int k = ORDER; k >= 1; k--) {
            const QString ctx = recent.mid(recent.size() - k, k).join(" ");
            cand = QiQuery<Gram>().filter(QiWhere("context = ", ctx)).all();
            if (cand.size() > 0) break;
        }

        QString next;
        if (cand.size() == 0) {
            // Never seen this context at all — pick any word so we can keep going.
            next = vocabList.isEmpty() ? "." : vocabList.at(rng.bounded(vocabList.size()));
        } else {
            // Weighted random pick. `temperature` reshapes the odds:
            //   low  (<1) -> lean toward the most common next word (predictable)
            //   high (>1) -> even out the odds (surprising)
            double total = 0.0;
            QVector<double> weights(cand.size());
            for (int j = 0; j < cand.size(); j++) {
                const int c = cand.at(j)->n;
                weights[j] = std::pow(double(c), 1.0 / qMax(0.01, temperature));
                total += weights[j];
            }
            double r = rng.generateDouble() * total;
            int pick = cand.size() - 1;
            for (int j = 0; j < cand.size(); j++) { r -= weights[j]; if (r <= 0.0) { pick = j; break; } }
            next = cand.at(pick)->word;
        }

        if (next == ".") {                       // end of a sentence
            out << ".";
            recent.clear();
            for (int i = 0; i < ORDER; i++) recent << ".";   // start a fresh sentence
        } else {
            out << next;
            recent << next;
            while (recent.size() > ORDER) recent.removeFirst();
            wordsWritten++;
        }
    }

    // -----------------------------------------------------------------
    // 4) Tidy the token stream into readable sentences.
    // -----------------------------------------------------------------
    QString text; bool sentenceStart = true;
    for (const QString &t : out) {
        if (t == ".") { text = text.trimmed() + ". "; sentenceStart = true; continue; }
        QString w = t;
        if (sentenceStart && !w.isEmpty()) { w[0] = w[0].toUpper(); sentenceStart = false; }
        text += w + " ";
    }
    text = text.trimmed();
    if (!text.endsWith('.')) text += ".";

    qInfo().noquote() << "\n" << text;

    connection.close();
    return 0;
}
