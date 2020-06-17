#-------------------------------------------------
#
# Project created by QtCreator 2018-11-20T09:11:47
#
#-------------------------------------------------

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


