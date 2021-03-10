CONFIG -= qt warn_on
CONFIG += exceptions_off rtti_off warn_off

TEMPLATE = lib

TARGET = png

unix:CONFIG += static

DEFINES +=  PNG_ARM_NEON_OPT=0 PNG_POWERPC_VSX_OPT=0
win32:DEF_FILE = symbols.def

include($${PWD}/../nwDeployed.pri)


HEADERS +=  \
    png.h \
    pngconf.h \
    pngdebug.h \
    pnginfo.h \
    pnglibconf.h \
    pngpriv.h \
    pngstruct.h


SOURCES += \
     png.c \
     pngerror.c \
     pngget.c \
     pngmem.c \
     pngpread.c \
     pngread.c \
     pngrio.c \
     pngrtran.c \
     pngrutil.c \
     pngset.c \
     pngtrans.c \
     pngwio.c \
     pngwrite.c \
     pngwtran.c \
     pngwutil.c \


INCLUDEPATH += ../zlib/include

LIBS += -lz


win32{
LIBFROM = $${DESTDIR}/$${TARGET}.lib
LIBTO = $${DESTDIR}/lib$${TARGET}.lib
LIBFROM = $$replace(LIBFROM, /, \\)
LIBTO = $$replace(LIBTO, /, \\)
QMAKE_POST_LINK += copy /Y $$LIBFROM $$LIBTO
}
