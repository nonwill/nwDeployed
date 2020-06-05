#-------------------------------------------------
#
# Project created by QtCreator 2011-07-04T16:26:46
#
#-------------------------------------------------

QT -= core gui

TARGET = hunspell
#TARGETTYPE=DLL
DEFINES += BUILDING_LIBHUNSPELL
#DEFINES += HUNSPELL_STATIC

TEMPLATE = lib
mac:CONFIG += static
CONFIG += precompile_header

include($${PWD}/../nwDeployed.pri)

INCLUDEPATH += . \
    src/hunspell \
    src/win_api

SOURCES += \
    src/hunspell/affentry.cxx \
    src/hunspell/affixmgr.cxx \
    src/hunspell/csutil.cxx \
    src/hunspell/filemgr.cxx \
    src/hunspell/hashmgr.cxx \
    src/hunspell/hunspell.cxx \
    src/hunspell/hunzip.cxx \
    src/hunspell/phonet.cxx \
    src/hunspell/replist.cxx \
    src/hunspell/suggestmgr.cxx \

HEADERS +=\
    src/hunspell/affentry.hxx \
    src/hunspell/affixmgr.hxx \
    src/hunspell/atypes.hxx \
    src/hunspell/baseaffix.hxx \
    src/hunspell/csutil.hxx \
    src/hunspell/filemgr.hxx \
    src/hunspell/hashmgr.h \
    src/hunspell/htypes.hxx \
    src/hunspell/hunspell.h \
    src/hunspell/hunspell.hxx \
    src/hunspell/hunzip.hxx \
    src/hunspell/langnum.hxx \
    src/hunspell/phonet.hxx \
    src/hunspell/replist.hxx \
    src/hunspell/suggestmgr.hxx \
    src/hunspell/w_char.hxx \
    src/hunspell/hunvisapi.h \
    src/win_api/config.h \

!static: {
  win32:RC_FILE = msvc/Hunspell.rc
}

#OTHER_FILES +=\
#    src/hunspell/license.myspell \
#    src/hunspell/license.hunspell \
#    src/hunspell/utf_info.cxx
