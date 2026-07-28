QT       += core sql
QT       -= gui

TARGET   = findsimilar
CONFIG   += c++17 console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += models.h corpus.h
SOURCES += main.cpp

# Pull in the Qivot library sources from the sibling repo (../../qivot).
include(../../qivot/src/qivot.pri)
