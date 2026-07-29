QT       += core sql
QT       -= gui

TARGET   = classify
CONFIG   += c++17 console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += models.h corpus.h
SOURCES += main.cpp

include(../../qivot/src/qivot.pri)
