CONFIG -= qt app_bundle

TEMPLATE = app
CONFIG += console

TARGET = html2text

DEFINES += AUTO_PTR_BROKEN

include($${PWD}/../nwDeployed.pri)
win32:INCLUDEPATH += $${PWD}/../include/win32 \
    $${PWD}/../libiconv/include

SOURCES +=  HTMLParser.tab.cc \
    Area.cpp \
    cmp_nocase.cpp \
    format.cpp \
    html.cpp \
    html2text.cpp \
    HTMLControl.cpp \
    HTMLDriver.cpp \
    Properties.cpp \
    sgml.cpp \
    table.cpp \
    iconvstream.cpp

HEADERS += HTMLParser.tab.hh \
    Area.h \
    cmp_nocase.h \
    format.h \
    html.h \
#    html2text.h \
    HTMLControl.h \
    HTMLDriver.h \
#    HTMLParser.h \
    Properties.h \
    sgml.h \
    iconvstream.h

LIBS += -liconv
