CONFIG -= qt warn_on
CONFIG += exceptions_off rtti_off warn_off

TEMPLATE = lib

TARGET = tiff

unix:CONFIG += static

DEFINES += DISABLE_CHECK_TIFFSWABMACROS HAS_LIBWEBP HAS_LIBJPEG HAS_ZLIB HAS_ZSTD
win32:DEFINES += TIF_PLATFORM_CONSOLE
win32:msvc*:DEFINES += inline=__inline
win32:DEF_FILE = libtiff.def

include($${PWD}/../nwDeployed.pri)


SOURCES += \
     tif_aux.c \
     tif_close.c \
     tif_codec.c \
     tif_color.c \
     tif_compress.c \
     tif_dir.c \
     tif_dirinfo.c \
     tif_dirread.c \
     tif_dirwrite.c \
     tif_dumpmode.c \
     tif_error.c \
     tif_extension.c \
     tif_fax3.c \
     tif_fax3sm.c \
     tif_flush.c \
     tif_getimage.c \
     tif_jbig.c \
     tif_jpeg.c \
#     tif_jpeg_12.c \
     tif_luv.c \
     tif_lzma.c \
     tif_lzw.c \
     tif_next.c \
     tif_ojpeg.c \
     tif_open.c \
     tif_packbits.c \
     tif_pixarlog.c \
     tif_predict.c \
     tif_print.c \
     tif_read.c \
     tif_strip.c \
     tif_swab.c \
     tif_thunder.c \
     tif_tile.c \
     tif_version.c \
     tif_warning.c \
     tif_webp.c \
     tif_write.c \
     tif_zip.c \
     tif_zstd.c

win32 {
    SOURCES += tif_win32.c
} else {
    SOURCES +=  tif_unix.c
}

win32:msvc* {
    SOURCES += \
        port/snprintf.c
}


HEADERS +=  \
    t4.h \
    tif_dir.h \
    tif_fax3.h \
    tiff.h \
    tiffconf.h \
    tiffio.h \
    tiffiop.h \
    tiffvers.h \
    tif_config.h \
    tif_predict.h \
    uvcode.h

INCLUDEPATH += $${PWD}/../libjpeg \
    $${PWD}/../libwebp/src \
    $${PWD}/../zlib/include \
    $${PWD}/../zstd/lib

LIBS += -ljpeg -lwebp -lz -lzstd
