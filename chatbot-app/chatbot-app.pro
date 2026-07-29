QT       += core gui network sql qml quick quickcontrols2

TARGET   = chatbotapp
CONFIG   += c++17
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += message.h chatservice.h
SOURCES += main.cpp chatservice.cpp
RESOURCES += qml.qrc

include(../../qivot/src/qivot.pri)
