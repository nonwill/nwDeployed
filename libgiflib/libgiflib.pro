CONFIG -= qt

TEMPLATE = lib

TARGET = giflib

CONFIG += static

include($${PWD}/../nwDeployed.pri)


HEADERS +=  \
    gif_hash.h \
    gif_lib.h \
    gif_lib_private.h


SOURCES += \
     dgif_lib.c \
     egif_lib.c \
     gifalloc.c \
     gif_err.c \
     gif_font.c \
     gif_hash.c \
     openbsd-reallocarray.c \
     quantize.c


INCLUDEPATH += ../zlib/include

#win32:msvc* {
#    LIBS += -lzlib
#} else {
    LIBS += -lz
#}
