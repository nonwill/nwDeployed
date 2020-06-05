CONFIG -= qt

TEMPLATE = lib

TARGET = sqlite3

DEFINES += SQLITE_API=__declspec\(dllexport\)  \
    SQLITE_THREADSAFE=1 \
    SQLITE_ENABLE_FTS3=1 \
    SQLITE_ENABLE_FTS4=1 \
    SQLITE_ENABLE_FTS5=1 \
    SQLITE_ENABLE_JSON1=1 \
    SQLITE_ENABLE_COLUMN_METADATA=1 \
    SQLITE_CORE=1

include($${PWD}/../nwDeployed.pri)

HEADERS += sqlite3.h

SOURCES += sqlite3.c

INCLUDEPATH += ./include
