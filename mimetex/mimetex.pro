CONFIG -= qt app_bundle

TEMPLATE = app

TARGET = mimetex

CONFIG += console

win32:DEFINES += WINDOWS
DEFINES += AA

include($${PWD}/../nwDeployed.pri)

SOURCES +=  \
    mimetex.c  \
    gifsave.c

HEADERS +=
