CONFIG -= qt

TEMPLATE = lib

TARGET = z

CONFIG += static

include($${PWD}/../nwDeployed.pri)


HEADERS +=  \
        include/crc32.h \
        include/deflate.h \
        include/gzguts.h \
        include/inffast.h \
        include/inffixed.h \
        include/inflate.h \
        include/inftrees.h \
        include/trees.h \
        include/zconf.h \
        include/zlib.h \
        include/zutil.h

SOURCES += \
        src/adler32.c \
        src/compress.c \
        src/crc32.c \
        src/deflate.c \
        src/gzclose.c \
        src/gzlib.c \
        src/gzread.c \
        src/gzwrite.c \
        src/infback.c \
        src/inffast.c \
        src/inflate.c \
        src/inftrees.c \
        src/trees.c \
        src/uncompr.c \
        src/zutil.c


INCLUDEPATH += ./include

