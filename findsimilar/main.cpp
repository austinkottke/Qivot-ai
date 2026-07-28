/** Find Similar — how an AI "looks things up," shown step by step.

    Big AI assistants feel like they understand your question. A huge part of that
    is a simple trick: turn everything into NUMBERS, then measure which numbers are
    closest. Close numbers = similar meaning. That's it.

    This program builds a tiny searchable library and shows every step on screen:

      STEP 1  the notes we can search
      STEP 2  how one note becomes a row of numbers (its "fingerprint")
      STEP 3  your search turned into numbers the same way
      STEP 4  compare fingerprints and rank the notes by closeness

    The fingerprints are stored as rows in a SQLite table (through Qivot) — which
    is exactly what a real search engine calls an "index."

    Run it:
        ./findsimilar                                  # uses a default search
        ./findsimilar teach my dog to sit              # your own words
        ./findsimilar stars and planets at night
        ./findsimilar a warm morning drink
 */
#include "models.h"
#include "corpus.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QStringList>
#include <QTextStream>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QChar>
#include <cmath>
#include <algorithm>

static QTextStream out(stdout);

// Tiny list of "filler" words that carry no topic (the, a, my, how...). Dropping
// them keeps the fingerprints about the actual subject.
static const QSet<QString> kStop = {
    "the","a","an","and","or","of","to","in","on","at","is","are","was","were",
    "it","its","this","that","for","with","as","by","be","i","you","my","your",
    "how","do","does","can","what","when","which","me","we","they","them","their",
    "from","into","out","up","down","so","if","then","them","also","until","through"
};

// Split text into lowercase topic-words (letters only, no filler words).
static QStringList topicWords(const QString &text) {
    QStringList res; QString w;
    auto flush = [&] { if (!w.isEmpty()) { if (!kStop.contains(w)) res << w; w.clear(); } };
    for (const QChar &c : text) { if (c.isLetter()) w += c.toLower(); else flush(); }
    flush();
    return res;
}

// A horizontal bar made of block characters, for a fraction from 0..1.
static QString bar(double frac, int width = 26) {
    frac = std::max(0.0, std::min(1.0, frac));
    const int n = int(std::lround(frac * width));
    return QString(n, QChar(0x2588)) + QString(width - n, QChar(0x2591));  // █ and ░
}

static QString pad(QString s, int w) {
    if (s.size() > w) s = s.left(w - 2) + "..";
    return s.leftJustified(w, ' ');
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif

    // The search text: everything after the program name, or a default.
    QString query = "how do i teach my puppy to sit and stay";
    if (argc > 1) {
        QStringList qs;
        for (int i = 1; i < argc; i++) qs << QString::fromLocal8Bit(argv[i]);
        query = qs.join(" ");
    }

    // --- database that holds the notes and their fingerprints ---
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("findsimilar.db");     // a real file you can open and inspect
    if (!db.open()) { out << "cannot open database\n"; return 1; }

    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<Doc>();
    connection.addModel<Term>();
    if (!connection.dropTables() || !connection.createTables()) return 1;

    // =================================================================
    // BUILD THE INDEX
    // A word that appears in almost every note (like "water") is not very
    // telling. A rare word (like "telescope") is very telling. "idf" gives rare
    // words a bigger number, so a note's fingerprint highlights what makes it special.
    // =================================================================
    QVector<QStringList> docWords(kDocCount);
    QHash<QString, int>  docFreq;              // in how many notes does each word appear?
    for (int i = 0; i < kDocCount; i++) {
        docWords[i] = topicWords(QString::fromUtf8(kDocs[i].text));
        QSet<QString> unique(docWords[i].begin(), docWords[i].end());
        for (const QString &w : unique) docFreq[w] += 1;
    }
    auto idf = [&](const QString &w) {
        return std::log(double(kDocCount) / (1.0 + docFreq.value(w, 0))) + 1.0;
    };

    QVector<int>    docId(kDocCount);
    QVector<double> docNorm(kDocCount);
    {
        QiTransaction tx;
        for (int i = 0; i < kDocCount; i++) {
            QHash<QString, int> tf;                       // word counts in this note
            for (const QString &w : docWords[i]) tf[w] += 1;

            double sumSq = 0.0;
            QHash<QString, double> weight;
            for (auto it = tf.constBegin(); it != tf.constEnd(); ++it) {
                const double wt = it.value() * idf(it.key());
                weight[it.key()] = wt;
                sumSq += wt * wt;
            }
            const double norm = std::sqrt(sumSq);

            Doc d; d.title = kDocs[i].title; d.text = kDocs[i].text; d.norm = norm;
            d.save();
            const int id = d.id.get().toInt();
            docId[i] = id; docNorm[i] = norm;

            for (auto it = weight.constBegin(); it != weight.constEnd(); ++it) {
                Term t; t.docId = id; t.word = it.key(); t.weight = it.value();
                t.save();
            }
        }
        tx.commit();
    }
    QHash<int, int> indexOf;                    // db id -> position 0..kDocCount-1
    for (int i = 0; i < kDocCount; i++) indexOf[docId[i]] = i;

    // =================================================================
    // STEP 1 — the library
    // =================================================================
    const QString rule(60, '=');
    out << "\n" << rule << "\n";
    out << "  FIND SIMILAR - searching by meaning, one step at a time\n";
    out << rule << "\n";
    out << "\nSTEP 1 - The notes we can search (" << kDocCount << " of them):\n\n";
    for (int i = 0; i < kDocCount; i++)
        out << "   [" << (i + 1) << "] " << kDocs[i].title << "\n";

    // =================================================================
    // STEP 2 — one note as numbers
    // =================================================================
    const int sample = 0;                       // show the first note as an example
    out << "\nSTEP 2 - A computer can't read words, so each note becomes NUMBERS.\n";
    out << "         Here is note [" << (sample + 1) << "] \"" << kDocs[sample].title
        << "\" as its top fingerprint words\n         (bigger bar = word that makes this note special):\n\n";
    {
        QiList<Term> tl = QiQuery<Term>()
                            .filter(QiWhere("docId = ", docId[sample]))
                            .orderBy("weight desc").all();
        const double top = tl.size() > 0 ? double(tl.at(0)->weight) : 1.0;
        for (int j = 0; j < tl.size() && j < 6; j++) {
            const QString w = tl.at(j)->word;
            const double  v = tl.at(j)->weight;
            out << "     " << pad(w, 12) << " " << bar(v / top, 20)
                << "  " << QString::number(v, 'f', 1) << "\n";
        }
    }

    // =================================================================
    // STEP 3 — the query as numbers
    // =================================================================
    const QStringList qWords = topicWords(query);
    QHash<QString, int> qtf;
    for (const QString &w : qWords) qtf[w] += 1;
    QHash<QString, double> qWeight; double qSumSq = 0.0;
    for (auto it = qtf.constBegin(); it != qtf.constEnd(); ++it) {
        const double wt = it.value() * idf(it.key());
        qWeight[it.key()] = wt; qSumSq += wt * wt;
    }
    const double qNorm = std::sqrt(qSumSq);

    out << "\nSTEP 3 - Your search, turned into numbers the exact same way:\n\n";
    out << "         \"" << query << "\"\n";
    out << "         keywords that matter: " << (qWords.isEmpty() ? QString("(none)") : qWords.join(", ")) << "\n";

    // =================================================================
    // STEP 4 — compare and rank
    // Look up each query word in the index, add up the overlap, then divide by the
    // fingerprint lengths (that division is "cosine similarity" — it just makes
    // the score fair regardless of how long each note is).
    // =================================================================
    QHash<int, double> dot;
    for (auto it = qWeight.constBegin(); it != qWeight.constEnd(); ++it) {
        QiList<Term> postings = QiQuery<Term>().filter(QiWhere("word = ", it.key())).all();
        for (int k = 0; k < postings.size(); k++)
            dot[int(postings.at(k)->docId)] += it.value() * double(postings.at(k)->weight);
    }

    struct Result { int i; double sim; };
    QVector<Result> ranked;
    for (int i = 0; i < kDocCount; i++) {
        const double d = dot.value(docId[i], 0.0);
        const double sim = (qNorm > 0 && docNorm[i] > 0) ? d / (qNorm * docNorm[i]) : 0.0;
        ranked.push_back({ i, sim });
    }
    std::sort(ranked.begin(), ranked.end(), [](const Result &a, const Result &b) { return a.sim > b.sim; });

    out << "\nSTEP 4 - How close is each note to your search? (longer bar = closer)\n\n";
    const double best = ranked.isEmpty() || ranked[0].sim <= 0 ? 1.0 : ranked[0].sim;
    for (int r = 0; r < ranked.size(); r++) {
        const Result &res = ranked[r];
        const QString star = (r == 0 && res.sim > 0) ? QString(">>") : QString("  ");
        out << " " << star << " " << pad(kDocs[res.i].title, 20) << "  "
            << bar(res.sim / best) << "  " << QString::number(res.sim, 'f', 2) << "\n";
    }

    out << "\n";
    if (ranked.isEmpty() || ranked[0].sim <= 0)
        out << "  No overlap found - try words that appear in the notes above.\n";
    else
        out << "  Best match: \"" << kDocs[ranked[0].i].title
            << "\". Nothing understood the words - it just found the closest numbers.\n";

    out << "\n  The fingerprints live in findsimilar.db. Peek at the index with:\n";
    out << "      sqlite3 findsimilar.db \"SELECT word, docId, round(weight,1) "
           "FROM term WHERE word='puppy';\"\n\n";

    connection.close();
    return 0;
}
