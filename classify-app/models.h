#ifndef MODELS_H
#define MODELS_H

#include <qivot.h>

/// For each word, how many times it appeared in each bucket. The whole "brain."
class WordCount : public QiModel {
    QI_MODEL
public:
    QiField<QString> word;
    QiField<int>     cls;   ///< 0 = normal, 1 = junk
    QiField<int>     n;     ///< times this word appeared in that bucket
};
QI_DECLARE_MODEL(WordCount, "wordcount",
    QI_FIELD(word), QI_FIELD(cls), QI_FIELD(n));

#endif // MODELS_H
