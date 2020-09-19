CONFIG -= qt

TEMPLATE = app

TARGET = dictzip

CONFIG += console

win32:DEFINES += DICTZIP_WIN32 DICT_VERSION=\\\"1.13-0-Win32\\\"

include($${PWD}/../nwDeployed.pri)

SOURCES +=  \
    data.c \
    dictzip.c\
    posix/getopt.c \
    posix/getopt1.c \
    posix/getopt_init.c

HEADERS += \
    data.h \
    defs.h \
    dictzip.h \
    posix/getopt.h \
    posix/getopt_int.h


INCLUDEPATH += ../zlib/include

LIBS += -lz
