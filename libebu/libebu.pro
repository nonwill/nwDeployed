#-------------------------------------------------
#
# Project created by QtCreator 2018-11-20T09:11:47
#
#-------------------------------------------------

QT       -= qt

TARGET = ebu
TEMPLATE = lib

CONFIG += static
#DEFINES += ENABLE_EBNET EB_ENABLE_EBNET ENABLE_WINSOCK2
#DEFINES += EB_ENABLE_PTHREAD ENABLE_PTHREAD

include($${PWD}/../nwDeployed.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS 
win32:DEFINES += HAVE_DIRECT_H DOS_FILE_PATH
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += ./eb
win32:INCLUDEPATH += ./win32

SOURCES += ebu/appendix.c \
    ebu/appsub.c \
    ebu/bcd.c \
    ebu/binary.c \
    ebu/bitmap.c \
    ebu/book.c \
    ebu/booklist.c \
    ebu/copyright.c \
    ebu/cross.c \
    ebu/eb.c \
    ebu/endword.c \
    ebu/error.c \
    ebu/exactword.c \
    ebu/filename.c \
    ebu/font.c \
    ebu/hook.c \
    ebu/jacode.c \
    ebu/keyword.c \
    ebu/lock.c \
    ebu/log.c \
    ebu/match.c \
    ebu/menu.c \
    ebu/multi.c \
    ebu/narwalt.c \
    ebu/narwfont.c \
    ebu/readtext.c \
    ebu/search.c \
    ebu/setword.c \
    ebu/stopcode.c \
    ebu/strcasecmp.c \
    ebu/subbook.c \
    ebu/text.c \
    ebu/widealt.c \
    ebu/widefont.c \
    ebu/word.c \
    ebu/utf8.c \
    ebu/zio.c

HEADERS += ebu/appendix.h \
    ebu/binary.h \
    ebu/booklist.h \
    ebu/defs.h \
    ebu/eb.h \
    ebu/error.h \
    ebu/font.h \
    ebu/text.h \
    ebu/zio.h


EBNET_SOURCES = ebu/ebnet.c \
    ebu/multiplex.c \
    ebu/linebuf.c \
    ebu/urlparts.c \
    ebu/getaddrinfo.c \
    ebu/dummyin6.c

contains(DEFINES, ENABLE_EBNET) {
SOURCES += $$EBNET_SOURCES
}

INCLUDEPATH += $${PWD}/../zlib/include
LIBS += -lz
