#ifndef MODELS_H
#define MODELS_H

#include <qivot.h>

/// The whole "brain" of the classifier: for each word, how many times it showed up
/// in each kind of message. Look these counts up later to score a new message.
///
///   word      cls   n
///   -------   ---   -
///   free       1    6      <- "free" appeared 6 times in JUNK  (cls 1)
///   free       0    0      <- ...and 0 times in NORMAL (cls 0)
///   meeting    0    3      <- "meeting" appeared 3 times in NORMAL
class WordCount : public QiModel {
    QI_MODEL
public:
    QiField<QString> word;
    QiField<int>     cls;   ///< 0 = normal, 1 = junk
    QiField<int>     n;     ///< times this word appeared in that class
};
QI_DECLARE_MODEL(WordCount, "wordcount",
    QI_FIELD(word), QI_FIELD(cls), QI_FIELD(n));

#endif // MODELS_H
