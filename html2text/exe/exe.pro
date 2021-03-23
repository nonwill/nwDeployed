CONFIG -= qt app_bundle

TEMPLATE = app
CONFIG += console

TARGET = html2text

DEFINES += HTML2TEXT_EXE

include(../html2text.pri)
