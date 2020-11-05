CONFIG -= qt

TEMPLATE = app

TARGET = mimetex

CONFIG += console

win32:DEFINES += WINDOWS AA

include($${PWD}/../nwDeployed.pri)

SOURCES +=  \
    mimetex.c  \
    gifsave.c

HEADERS +=
