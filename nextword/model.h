#ifndef MODEL_H
#define MODEL_H

#include <qivot.h>

/// One thing the model learned from the training text: after the word(s) in
/// `context`, the word `word` was seen `n` times. That's the whole "brain" —
/// a pile of these rows in a SQLite table.
///
///   context     word     n
///   ---------   ------   ---
///   "the"       "cat"     4
///   "the"       "sea"     7
///   "the cat"   "sleeps"  3
///
/// To guess what comes after "the cat", we just read the rows whose context is
/// "the cat" and pick one of the words — favoring the ones with a bigger `n`.
class Gram : public QiModel {
    QI_MODEL
public:
    QiField<QString> context;   ///< the previous word, or two words joined by a space
    QiField<QString> word;      ///< a word that was seen right after `context`
    QiField<int>     n;         ///< how many times that happened in the training text
};

QI_DECLARE_MODEL(Gram, "gram",
    QI_FIELD(context),
    QI_FIELD(word),
    QI_FIELD(n));

#endif // MODEL_H
