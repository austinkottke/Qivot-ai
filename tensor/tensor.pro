QT       += core sql
QT       -= gui

TARGET   = tensor
CONFIG   += c++17 console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += models.h
SOURCES += main.cpp

include(../../qivot/src/qivot.pri)
