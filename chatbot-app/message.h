#ifndef MESSAGE_H
#define MESSAGE_H

#include <qivot.h>

/// One line of the conversation, stored in SQLite so your chat history survives
/// across launches. This is the honest "Qivot" part of this lesson: the model does
/// the talking, Qivot does the remembering.
class Message : public QiModel {
    QI_MODEL
public:
    QiField<int>     role;       ///< 0 = you, 1 = the assistant
    QiField<QString> text;
    QiField<QString> createdAt;
};
QI_DECLARE_MODEL(Message, "message",
    QI_FIELD(role), QI_FIELD(text), QI_FIELD(createdAt));

#endif // MESSAGE_H
