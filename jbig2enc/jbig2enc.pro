CONFIG -= qt

TEMPLATE = app

TARGET = jbig2enc

CONFIG += console

DEFINES += VERSION=\\\"0\.28\\\"
win32::msvc*:QMAKE_CFLAGS += /TP # Compile C as CPlusPlus, this is important.

include($${PWD}/../nwDeployed.pri)

HEADERS += \
    pdf.h \
    src/jbig2arith.h \
    src/jbig2comparator.h \
    src/jbig2enc.h \
    src/jbig2segments.h \
    src/jbig2structs.h \
    src/jbig2sym.h

SOURCES += \
    src/jbig2.cc \
    src/jbig2arith.cc \
    src/jbig2comparator.cc \
    src/jbig2enc.cc \
    src/jbig2sym.cc

INCLUDEPATH += $${PWD}/../libleptonica/src .

LIBS += -lliblept

