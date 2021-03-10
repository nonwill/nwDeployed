CONFIG -= qt warn_on

TEMPLATE = lib

TARGET = jpeg

unix:CONFIG += static
win32:DEF_FILE = jpeg8.def

CONFIG += exceptions_off rtti_off warn_off

include($${PWD}/../nwDeployed.pri)


HEADERS +=  \
    jconfig.h \
    jdct.h \
    jerror.h \
    jinclude.h \
    jmemsys.h \
    jmorecfg.h \
    jpegint.h \
    jpeglib.h \
    jversion.h \


SOURCES += \
     jaricom.c \
     jcapimin.c \
     jcapistd.c \
     jcarith.c \
     jccoefct.c \
     jccolor.c \
     jcdctmgr.c \
     jchuff.c \
     jcinit.c \
     jcmainct.c \
     jcmarker.c \
     jcmaster.c \
     jcomapi.c \
     jcparam.c \
     jcprepct.c \
     jcsample.c \
     jctrans.c \
     jdapimin.c \
     jdapistd.c \
     jdarith.c \
     jdatadst.c \
     jdatasrc.c \
     jdcoefct.c \
     jdcolor.c \
     jddctmgr.c \
     jdhuff.c \
     jdinput.c \
     jdmainct.c \
     jdmarker.c \
     jdmaster.c \
     jdmerge.c \
     jdpostct.c \
     jdsample.c \
     jdtrans.c \
     jerror.c \
     jfdctflt.c \
     jfdctfst.c \
     jfdctint.c \
     jidctflt.c \
     jidctfst.c \
     jidctint.c \
     jquant1.c \
     jquant2.c \
     jutils.c \
     jmemmgr.c \
     jmemnobs.c


INCLUDEPATH +=

LIBS +=


win32{
LIBFROM = $${DESTDIR}/jpeg.lib
LIBTO = $${DESTDIR}/libjpeg.lib
LIBFROM = $$replace(LIBFROM, /, \\)
LIBTO = $$replace(LIBTO, /, \\)
QMAKE_POST_LINK += copy /Y $$LIBFROM $$LIBTO
}
