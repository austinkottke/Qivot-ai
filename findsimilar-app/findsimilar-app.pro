QT       += core sql qml quick quickcontrols2

TARGET   = findsimilarapp
CONFIG   += c++17
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += models.h corpus.h searchmodel.h
SOURCES += main.cpp searchmodel.cpp
RESOURCES += qml.qrc

wasm: QTPLUGIN += qsqlite

include(../../qivot/src/qivot.pri)
