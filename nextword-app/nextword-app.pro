QT       += core sql qml quick quickcontrols2

# The wasm module's JS entry point is derived from TARGET and must be a valid
# identifier, so keep it hyphen-free (handy if we compile this to WebAssembly later).
TARGET   = nextwordapp
CONFIG   += c++17
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += model.h corpus.h wordmodel.h
SOURCES += main.cpp wordmodel.cpp
RESOURCES += qml.qrc

# WebAssembly is a static build, so the SQLite driver must be linked in explicitly.
wasm: QTPLUGIN += qsqlite

include(../../qivot/src/qivot.pri)
