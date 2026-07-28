/** Tensors — the numbers AI is made of.

    Everything inside a neural network is a "tensor" — just a box of numbers:
      a single number (scalar), a row of numbers (vector), a grid (matrix), ...

    And the one move a network makes, over and over, is: take your input numbers,
    MULTIPLY them by a grid of weights, add a nudge, and squash the result. That's
    one "layer." This program runs exactly one layer and shows every number.

    The example: turn a COLOR (3 numbers: red, green, blue) into 2 new numbers we've
    named "brightness" and "warmth". The weight grid is set by hand so you can see
    the math — a real network would LEARN these numbers from data. The grid lives in
    a SQLite table (through Qivot).

    Run it:
        ./tensor                 # a default orange
        ./tensor 30 90 200       # a blue  (R G B, each 0..255)
        ./tensor 250 250 250     # near white
 */
#include "models.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <cmath>

static QTextStream out(stdout);

// Squasher: turns ANY number into something between 0 and 1. Networks use this
// (an "activation") so outputs stay in a tidy range.
static double squash(double z) { return 1.0 / (1.0 + std::exp(-z)); }

static QString bar(double frac, int width = 22) {
    frac = std::max(0.0, std::min(1.0, frac));
    const int n = int(std::lround(frac * width));
    return QString(n, QChar(0x2588)) + QString(width - n, QChar(0x2591));  // block + light
}
static QString num(double v) { return QString::asprintf("%+6.2f", v); }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif

    // Input color (0..255), default a warm orange.
    int R = 240, G = 150, B = 60;
    if (argc > 3) { R = QString(argv[1]).toInt(); G = QString(argv[2]).toInt(); B = QString(argv[3]).toInt(); }

    // --- database that holds the weights ---
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tensor.db");
    if (!db.open()) { out << "cannot open database\n"; return 1; }
    QiConnection connection;
    if (!connection.open(db)) return 1;
    connection.addModel<Weight>();
    connection.addModel<Bias>();
    if (!connection.dropTables() || !connection.createTables()) return 1;

    // The knowledge: a 2x3 weight grid + 2 biases. (Hand-picked, not learned.)
    //            R      G      B
    //  bright  0.30   0.59   0.11     (how much each color adds to "brightness")
    //  warm    0.90   0.00  -0.90     (red adds warmth, blue removes it)
    const char *outName[2] = { "brightness", "warmth" };
    const double W[2][3] = { { 0.30, 0.59, 0.11 }, { 0.90, 0.00, -0.90 } };
    const double Bs[2]   = { 0.0, 0.0 };
    {
        QiTransaction tx;
        for (int o = 0; o < 2; o++) {
            for (int i = 0; i < 3; i++) { Weight w; w.outIdx = o; w.inIdx = i; w.value = W[o][i]; w.save(); }
            Bias b; b.idx = o; b.value = Bs[o]; b.save();
        }
        tx.commit();
    }

    // Normalize the color to 0..1 — nicer numbers to look at.
    const double x[3] = { R / 255.0, G / 255.0, B / 255.0 };
    const char *inName[3] = { "red", "green", "blue" };

    const QString rule(60, '=');
    out << "\n" << rule << "\n";
    out << "  TENSORS - the numbers AI is made of\n";
    out << rule << "\n";

    out << "\nA tensor is just a box of numbers:\n";
    out << "   scalar (one number) :  5\n";
    out << "   vector (a row)      :  [ red green blue ]   <- our input below\n";
    out << "   matrix (a grid)     :  the weight grid in STEP 2\n";

    // STEP 1 — input vector
    out << "\nSTEP 1 - Your input color as a vector of 3 numbers (scaled to 0..1):\n\n";
    for (int i = 0; i < 3; i++)
        out << "     " << QString(inName[i]).leftJustified(7) << bar(x[i]) << "  "
            << QString::number(x[i], 'f', 2) << "\n";

    // STEP 2 — the weight grid, read back from the database
    out << "\nSTEP 2 - The weight grid: a 2x3 matrix (read from the SQLite table):\n\n";
    out << "                 red     green    blue\n";
    for (int o = 0; o < 2; o++) {
        out << "     " << QString(outName[o]).leftJustified(11);
        for (int i = 0; i < 3; i++) {
            QiList<Weight> wl = QiQuery<Weight>()
                                  .filter(QiWhere("outIdx = ", o) && QiWhere("inIdx = ", i)).all();
            const double v = wl.size() > 0 ? double(wl.at(0)->value) : 0.0;
            out << "  " << num(v);
        }
        out << "\n";
    }

    // STEP 3 — the multiply-and-add (this IS a neural layer)
    out << "\nSTEP 3 - Multiply the input by each row, and add them up:\n\n";
    double raw[2];
    for (int o = 0; o < 2; o++) {
        double sum = Bs[o];
        out << "     " << QString(outName[o]).leftJustified(11) << " = ";
        for (int i = 0; i < 3; i++) {
            sum += W[o][i] * x[i];
            out << QString::asprintf("%.2f*%.2f", x[i], W[o][i]);
            out << (i < 2 ? " + " : "");
        }
        raw[o] = sum;
        out << "  =  " << QString::number(sum, 'f', 2) << "\n";
    }

    // STEP 4 — squash + result
    out << "\nSTEP 4 - Squash each result to 0..1 (the 'activation'), and you get\n";
    out << "         a NEW vector of 2 numbers - the layer's output:\n\n";
    for (int o = 0; o < 2; o++) {
        const double y = squash(raw[o]);
        out << "     " << QString(outName[o]).leftJustified(11) << bar(y) << "  "
            << QString::number(y, 'f', 2) << "\n";
    }

    out << "\n  That multiply-a-grid-by-numbers step IS one layer of a neural network.\n";
    out << "  Stack many layers and let a computer tune the weights from examples\n";
    out << "  (instead of us setting them) - that's 'deep learning'.\n";

    out << "\n  The weights are just rows. See them with:\n";
    out << "      sqlite3 tensor.db \"SELECT outIdx, inIdx, value FROM weight;\"\n\n";

    connection.close();
    return 0;
}
