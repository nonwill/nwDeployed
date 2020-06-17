
QT       -= qt

TARGET = oss
TEMPLATE = lib

DEFINES += AO_BUILDING_LIBAO
u#nix:DEFINES += HAVE_DLFCN_H HAVE_DLOPEN

include($${PWD}/../../../../nwDeployed.pri)

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lao

INCLUDEPATH += ../../../include

SOURCES += ao_oss.c


