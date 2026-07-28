# Qivot AI

Small, friendly tools that show **how AI actually works** — by building the ideas
from scratch, in plain C++, on top of [Qivot](../qivot) (a database library).

No accounts, no API keys, no calling out to ChatGPT or anything else. Every program
here does its "thinking" with nothing but data in a local SQLite database. The goal
is to *demystify* AI: once you see the moving parts, a lot of the mystery goes away.

Each folder is one runnable lesson with a plain-English walkthrough. You don't need
to know any AI to follow along.

## Lessons

| Folder | What you build | The idea it teaches |
|--------|----------------|---------------------|
| [**nextword**](nextword/) | A tiny "guess the next word" writer | How a chatbot writes: guess the next word, over and over. Its whole "brain" is just rows in a table you can read. |

*(More on the way — a "search by meaning" tool and a "sort text into buckets" tool are next.)*

## What you need

- **Qt** 5.15 or 6 (with `qmake`)
- The **[`qivot`](../qivot)** repo checked out right next to this folder, so the layout is:

  ```
  dev/
    qivot/        # the database library
    qivot-ai/     # this project
  ```

## Build & run everything

```bash
cd qivot-ai
qmake && make          # builds all lessons
./nextword/nextword    # run one
```

Or build a single lesson from its own folder — see that folder's README.

## Why build on a database?

A surprising amount of "AI" is really just **storing things and looking them up
cleverly** — counts, word lists, numbers that measure how similar two things are.
A database is exactly the right tool for that, so each lesson stays short and the
AI idea stays front and center. Qivot handles the storage in a line or two, and we
spend our attention on the concept.
