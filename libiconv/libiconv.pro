CONFIG -= qt

TEMPLATE = lib

TARGET = iconv

win32:DEFINES += _WIN32 UNICODE _UNICODE
win32:DEFINES += BUILDING_LIBICONV BUILDING_DLL IN_LIBRARY
win32:DEFINES += PIC HAVE_CONFIG_H \
    NO_XMALLOC ENABLE_RELOCATABLE=1
win32:DEFINES += set_relocation_prefix=libiconv_set_relocation_prefix \
    relocate=libiconv_relocate
win32:DEFINES += INSTALLPREFIX=\\\"\\\" INSTALLDIR=\\\"\\\" LIBDIR=\\\"\\\"

include($${PWD}/../nwDeployed.pri)

INCLUDEPATH += $${PWD}/../include \
    ./lib \
    ./include

HEADERS += \
    config.h \
    include/iconv.h \
    lib/localcharset.h

SOURCES += \
    lib/iconv.c \
    libcharset/lib/localcharset.c \
    lib/relocatable.c


win32:RC_FILE += windows/libiconv.rc

LIBS +=
