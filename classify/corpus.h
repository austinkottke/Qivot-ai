#ifndef CORPUS_H
#define CORPUS_H

// The examples we learn from. Each message is already LABELLED — that's what makes
// this "supervised" learning: we show the computer answers, and it works out the
// pattern. Swap in your own examples (product reviews? support tickets?) and rebuild.

// JUNK (spammy) messages — class 1
static const char *kJunk[] = {
    "Congratulations you are our lucky winner claim your free prize now",
    "Free money click here to win a huge cash reward today",
    "You have won a free gift card claim it now before it expires",
    "Urgent act now to claim your prize winner selected click the link",
    "Get rich quick with this amazing free offer for a limited time only",
    "Winner winner you won the lottery send your details to claim the cash",
    "Free trial click now for a special discount offer just for you",
    "Claim your reward now this exclusive free bonus offer expires today",
};

// NORMAL (real) messages — class 0
static const char *kNormal[] = {
    "Hey are we still meeting for lunch tomorrow at noon",
    "Can you send me the report before the meeting this afternoon",
    "Thanks for your help yesterday the project is going really well",
    "Lets schedule a call to discuss the new design next week",
    "I will be a few minutes late to the office this morning",
    "Did you get a chance to review the document I shared with the team",
    "Happy birthday hope you have a wonderful day with your family",
    "The team lunch is moved to Friday so please update your calendar",
};

static const int kJunkCount   = int(sizeof(kJunk)   / sizeof(kJunk[0]));
static const int kNormalCount = int(sizeof(kNormal) / sizeof(kNormal[0]));

#endif // CORPUS_H
