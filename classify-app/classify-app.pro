QT       += core sql qml quick quickcontrols2

TARGET   = classifyapp
CONFIG   += c++17
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += models.h corpus.h classifier.h
SOURCES += main.cpp classifier.cpp
RESOURCES += qml.qrc

wasm: QTPLUGIN += qsqlite

include(../../qivot/src/qivot.pri)
