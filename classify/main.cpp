/** Sort into buckets — teaching a computer to spot junk mail.

    This is "classification": show the computer labelled examples (these are junk,
    these are normal), let it learn which words lean which way, then let it sort a
    NEW message it has never seen. The method is a classic called Naive Bayes, and
    it's just counting + a little probability.

      STEP 1  learn from the labelled examples
      STEP 2  see which words lean "junk" vs "normal"
      STEP 3  a new message
      STEP 4  add up the evidence word by word, and decide

    The learned counts are stored as rows in SQLite (through Qivot).

    Run it:
        ./classify                              # a default junky message
        ./classify can we meet for lunch friday # your own words -> NORMAL
        ./classify free cash prize click now    #                 -> JUNK
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
#include <cmath>
#include <algorithm>

static QTextStream out(stdout);

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

// A diverging bar: fills right for "junk" (frac > 0), left for "normal" (frac < 0).
static QString divBar(double frac) {
    const int H = 12;
    frac = std::max(-1.0, std::min(1.0, frac));
    const int n = int(std::lround(std::fabs(frac) * H));
    QString left(H, ' '), right(H, ' ');
    const QChar blk(0x2588);
    if (frac >= 0) { for (int k = 0; k < n; k++) right[k] = blk; }
    else           { for (int k = 0; k < n; k++) left[H - 1 - k] = blk; }
    return left + "|" + right;
}
static QString pad(QString s, int w) { return s.leftJustified(w, ' '); }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif

    QString query = "congratulations claim your free cash prize winner click now";
    if (argc > 1) {
        QStringList qs; for (int i = 1; i < argc; i++) qs << QString::fromLocal8Bit(argv[i]);
        query = qs.join(" ");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("classify.db");
    if (!db.open()) { out << "cannot open database\n"; return 1; }
    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<WordCount>();
    if (!connection.dropTables() || !connection.createTables()) return 1;

    // ---------------- TRAIN: count words per class ----------------
    QHash<QString, QVector<int>> cnt;          // cnt[word] = { normalCount, junkCount }
    int totalWords[2] = { 0, 0 };
    QSet<QString> vocab;
    auto learn = [&](const char *msg, int cls) {
        for (const QString &w : words(QString::fromUtf8(msg))) {
            if (!cnt.contains(w)) cnt[w] = QVector<int>{ 0, 0 };
            cnt[w][cls]++; totalWords[cls]++; vocab.insert(w);
        }
    };
    for (int i = 0; i < kNormalCount; i++) learn(kNormal[i], 0);
    for (int i = 0; i < kJunkCount;   i++) learn(kJunk[i],   1);

    // Save the learned counts into the database.
    {
        QiTransaction tx;
        for (auto it = cnt.constBegin(); it != cnt.constEnd(); ++it)
            for (int c = 0; c < 2; c++)
                if (it.value()[c] > 0) {
                    WordCount wc; wc.word = it.key(); wc.cls = c; wc.n = it.value()[c];
                    wc.save();
                }
        tx.commit();
    }

    const int V = vocab.size();
    const int docCount[2] = { kNormalCount, kJunkCount };
    const int totalDocs = docCount[0] + docCount[1];

    // P(word | class) with "+1" smoothing so an unseen word doesn't zero everything out.
    auto logP = [&](int cls, int wordCount) {
        return std::log((wordCount + 1.0) / (totalWords[cls] + V));
    };
    // How strongly a word leans junk (>0) vs normal (<0).
    auto lean = [&](const QString &w) {
        const QVector<int> c = cnt.value(w, QVector<int>{ 0, 0 });
        return logP(1, c[1]) - logP(0, c[0]);
    };

    const QString rule(60, '=');
    out << "\n" << rule << "\n";
    out << "  SORT INTO BUCKETS - is a message junk, or normal?\n";
    out << rule << "\n";

    // STEP 1
    out << "\nSTEP 1 - Learn from labelled examples:\n\n";
    out << "     " << kJunkCount << " junk + " << kNormalCount << " normal messages   ->   "
        << V << " different words learned\n";

    // STEP 2
    out << "\nSTEP 2 - Which words lean which way? (learned from the examples)\n\n";
    out << "                    normal <----------|----------> junk\n";
    QVector<QPair<QString, double>> leans;
    for (const QString &w : vocab) {
        const QVector<int> c = cnt.value(w);
        if (c[0] + c[1] >= 2) leans.push_back({ w, lean(w) });
    }
    std::sort(leans.begin(), leans.end(), [](auto &a, auto &b) { return a.second > b.second; });
    auto showLean = [&](const QString &w, double l) {
        out << "     " << pad(w, 12) << " " << divBar(l / 2.5) << "\n";
    };
    for (int i = 0; i < leans.size() && i < 4; i++) showLean(leans[i].first, leans[i].second);
    out << "     " << pad("...", 12) << "\n";
    for (int i = qMax(4, leans.size() - 4); i < leans.size(); i++) showLean(leans[i].first, leans[i].second);

    // STEP 3
    out << "\nSTEP 3 - A new message the computer has never seen:\n\n";
    out << "     \"" << query << "\"\n";

    // STEP 4 — score it, word by word (looking each word up in the database)
    out << "\nSTEP 4 - Weigh the evidence, word by word:\n\n";
    out << "                    normal <----------|----------> junk\n";
    double score[2] = { std::log(docCount[0] / double(totalDocs)),
                        std::log(docCount[1] / double(totalDocs)) };
    for (const QString &w : words(query)) {
        QiList<WordCount> rows = QiQuery<WordCount>().filter(QiWhere("word = ", w)).all();
        int wc[2] = { 0, 0 };
        for (int k = 0; k < rows.size(); k++) wc[int(rows.at(k)->cls)] = int(rows.at(k)->n);
        score[0] += logP(0, wc[0]);
        score[1] += logP(1, wc[1]);
        const double l = logP(1, wc[1]) - logP(0, wc[0]);
        out << "     " << pad(w, 12) << " " << divBar(l / 2.5) << "\n";
    }

    // decide
    const double pJunk = 1.0 / (1.0 + std::exp(score[0] - score[1]));
    const bool junk = pJunk >= 0.5;
    const int conf = int(std::lround((junk ? pJunk : 1 - pJunk) * 100));
    out << "\n     VERDICT:  " << (junk ? "JUNK" : "NORMAL")
        << "   (" << conf << "% sure)\n";
    out << "\n  It didn't 'read' anything - it just added up which words lean which way.\n";

    out << "\n  The learned brain is just rows. Look at 'free' vs 'meeting':\n";
    out << "      sqlite3 classify.db \"SELECT * FROM wordcount WHERE word IN ('free','meeting');\"\n\n";

    connection.close();
    return 0;
}
