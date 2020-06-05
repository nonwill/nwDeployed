CONFIG -= qt

TEMPLATE = lib

TARGET = bz2

CONFIG += static

include($${PWD}/../nwDeployed.pri)

win32:msvc*:DEFINES += _FILE_OFFSET_BITS=64

HEADERS +=  \
    bzlib.h \
    bzlib_private.h

SOURCES += \
    blocksort.c  \
    huffman.c    \
    crctable.c   \
    randtable.c  \
    compress.c   \
    decompress.c \
    bzlib.c

win32:DEF_FILE = libbz2.def
