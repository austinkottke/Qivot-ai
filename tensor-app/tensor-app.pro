QT       += core sql qml quick quickcontrols2

TARGET   = tensorapp
CONFIG   += c++17
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += models.h layer.h
SOURCES += main.cpp layer.cpp
RESOURCES += qml.qrc

wasm: QTPLUGIN += qsqlite

include(../../qivot/src/qivot.pri)
