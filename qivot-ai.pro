# Qivot AI — small tools + tutorials that teach how AI works, by building the
# ideas from scratch on top of Qivot (SQLite / tables / queries). No external
# AI service is ever contacted.
#
# Each subfolder is one runnable lesson:
#   nextword          - a tiny "guess the next word" language model (terminal)
#   nextword-app      - the same model as an interactive QML app (watch it think)
#   findsimilar       - search by meaning: turn text into numbers, measure closeness
#   findsimilar-app   - the same search as an interactive QML app (type and watch)
#   tensor            - the numbers a neural network is made of, one layer by hand
#   tensor-app        - the same layer as an interactive QML app (drag the sliders)
#   classify          - sort text into buckets (junk vs normal), learned from examples
#   classify-app      - the same detector as an interactive QML app (type and watch)
#   chatbot-app       - BONUS: a real downloaded model (Qwen 0.5B) running locally via llama.cpp

TEMPLATE = subdirs

SUBDIRS += \
    nextword \
    nextword-app \
    findsimilar \
    findsimilar-app \
    tensor \
    tensor-app \
    classify \
    classify-app \
    chatbot-app
