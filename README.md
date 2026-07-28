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
| [**nextword**](nextword/) | A tiny "guess the next word" writer (terminal) | How a chatbot writes: guess the next word, over and over. Its whole "brain" is just rows in a table you can read. |
| [**nextword-app**](nextword-app/) | The same model as a clickable **app** — watch it think | The same idea, but you *see* it: press Play and watch the choices and dice-rolls happen live. |
| [**findsimilar**](findsimilar/) | A step-by-step "search by meaning" tool | How an AI looks things up: turn text into numbers, then find the closest numbers. The heart of search and of RAG. |
| [**findsimilar-app**](findsimilar-app/) | The search as a clickable **app** — type and watch | The same idea, live: notes re-sort with animated bars as you type, and it shows *why* the winner matched. |
| [**tensor**](tensor/) | One neural-network layer, by hand | What AI is literally made of: "tensors" (boxes of numbers) and one move — multiply by a grid of weights. |
| [**tensor-app**](tensor-app/) | The layer as a clickable **app** — drag the sliders | The same layer, live: move R/G/B and watch the grid, the multiply-add, and the outputs recompute. |

*(More on the way — a "sort text into buckets" classifier is next.)*

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
