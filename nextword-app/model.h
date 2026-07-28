#ifndef MODEL_H
#define MODEL_H

#include <qivot.h>

/// One thing the model learned: after the word(s) in `context`, the word `word`
/// was seen `n` times. The whole "brain" is a pile of these rows in SQLite.
class Gram : public QiModel {
    QI_MODEL
public:
    QiField<QString> context;
    QiField<QString> word;
    QiField<int>     n;
};

QI_DECLARE_MODEL(Gram, "gram",
    QI_FIELD(context),
    QI_FIELD(word),
    QI_FIELD(n));

#endif // MODEL_H
