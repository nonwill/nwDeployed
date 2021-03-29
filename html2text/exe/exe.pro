CONFIG -= qt app_bundle

TEMPLATE = app
CONFIG += console

TARGET = html2text

DEFINES += HTML2TEXT_EXE

include(../html2text.pri)
win32:INCLUDEPATH += $${PWD} $${PWD}/../../include/win32 \
    $${PWD}/../../libiconv/include


SOURCES += $${PWD}/../iconvstream.cpp
HEADERS += $${PWD}/../iconvstream.h

LIBS += -liconv
