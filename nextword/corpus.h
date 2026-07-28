#ifndef CORPUS_H
#define CORPUS_H

// The training text. This is ALL the model ever "reads" — it never sees the
// internet, never talks to any AI service. It just learns the word patterns in
// these sentences. Lots of repetition on purpose, so there are interesting
// choices to make (e.g. after "good night" it could pick sea, sun, boat, or cat).
//
// Want different-sounding output? Replace this text with your own — a song, a
// recipe, your notes — rebuild, and the model will babble in that style instead.
static const char *kCorpus = R"(
The sea is calm today. The sea is calm and the sun is bright.
The little boat sails on the calm sea. The little boat sails far away.
The cat sleeps on the warm sand. The cat sleeps all day in the sun.
The children play near the sea. The children play and laugh in the sun.
The waves roll on the sand. The waves roll and the wind blows soft.
The old man walks by the sea. The old man walks and watches the little boat.
The gulls fly over the waves. The gulls fly and call to the wind.
The sun sets slow and red. The sun sets and the sea turns dark.
The stars come out one by one. The stars come out over the quiet sea.
The little boat comes home at night. The little boat comes home to the warm light.
The cat wakes and stretches in the dark. The cat wakes and walks to the door.
The children sleep and dream of the sea. The children sleep and the house is still.
Good night sea. Good night sun. Good night little boat. Good night cat.
The morning comes bright and new. The morning comes and the sea is calm again.
)";

#endif // CORPUS_H
