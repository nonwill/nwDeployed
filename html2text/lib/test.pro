QT += core gui

TEMPLATE = app
CONFIG += console

TARGET = html2text_test

include($${PWD}/../../nwDeployed.pri)

INCLUDEPATH += $${PWD}/..

SOURCES = html2text_test.cxx

LIBS += -lhtml2text
