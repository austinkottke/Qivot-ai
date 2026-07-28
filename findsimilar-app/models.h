#ifndef MODELS_H
#define MODELS_H

#include <qivot.h>

/// One note in the library. `norm` is the "length" of its number-fingerprint,
/// stored so comparisons are fair between long and short notes.
class Doc : public QiModel {
    QI_MODEL
public:
    QiField<QString> title;
    QiField<QString> text;
    QiField<double>  norm;
};
QI_DECLARE_MODEL(Doc, "doc",
    QI_FIELD(title), QI_FIELD(text), QI_FIELD(norm));

/// One number in a note's fingerprint: how important `word` is in note `docId`.
/// A table of these is exactly a search engine's "inverted index."
class Term : public QiModel {
    QI_MODEL
public:
    QiField<int>     docId;
    QiField<QString> word;
    QiField<double>  weight;
};
QI_DECLARE_MODEL(Term, "term",
    QI_FIELD(docId), QI_FIELD(word), QI_FIELD(weight));

#endif // MODELS_H
