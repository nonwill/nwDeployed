CONFIG -= qt

TEMPLATE = lib

TARGET = openjpeg

CONFIG += static

DEFINES +=  OPJ_STATIC
win32:msvc*:DEFINES += __SSE__ __SSE2__ __AVX__ __AVX2__ __FMA__ __SSE4_1__

include($${PWD}/../nwDeployed.pri)


HEADERS +=  \
    bio.h \
    cio.h \
    dwt.h \
    event.h \
    function_list.h \
    image.h \
    indexbox_manager.h \
    invert.h \
    j2k.h \
    jp2.h \
    mct.h \
    mqc.h \
    mqc_inl.h \
    openjpeg.h \
    opj_clock.h \
    opj_codec.h \
    opj_common.h \
    opj_config.h \
    opj_config_private.h \
    opj_includes.h \
    opj_intmath.h \
    opj_inttypes.h \
    opj_malloc.h \
    opj_stdint.h \
    pi.h \
    sparse_array.h \
    t1.h \
    t1_luts.h \
    t2.h \
    tcd.h \
    tgt.h \
    thread.h \
    tls_keys.h \



SOURCES += \
     bio.c \
     cio.c \
     dwt.c \
     event.c \
     function_list.c \
     image.c \
     invert.c \
     j2k.c \
     jp2.c \
     mct.c \
     mqc.c \
     openjpeg.c \
     opj_clock.c \
     opj_malloc.c \
     pi.c \
     sparse_array.c \
     t1.c \
     t2.c \
     tcd.c \
     tgt.c \
     thread.c



INCLUDEPATH +=

