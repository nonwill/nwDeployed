#-------------------------------------------------
#
# Project created by QtCreator 2018-11-20T09:11:47
#
#-------------------------------------------------

QT       -= qt

TARGET = ao
TEMPLATE = lib

mac:CONFIG += static

DEFINES += AO_BUILDING_LIBAO
unix:DEFINES += HAVE_DLFCN_H HAVE_DLOPEN HAVE_AU_UNIX
linux:DEFINES += SHARED_LIB_EXT=\\\".so\\\"
mac:DEFINES += SHARED_LIB_EXT=\\\".dylib\\\"
win32:DEFINES += HAVE_WMM

include($${PWD}/../nwDeployed.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += ./include ./include/ao

SOURCES += \
        src/config.c \
        src/ao_null.c \
        src/audio_out.c

win32:SOURCES += src/ao_wmm.c  src/dllmain.c
unix:SOURCES += src/ao_au.c


HEADERS += \
        ./include/ao/ao.h \
        ./include/ao/os_types.h \
        ./include/ao/ao_private.h

win32:LIBS += -lksuser -lwinmm
unix:LIBS += -ldl

