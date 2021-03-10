TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    zlib \
    libbzip2 \
    xz \
    mimetex

win32:SUBDIRS += \
    speex \
    libiconv \
    libxml2 \
    libxslt \
    sqlite3 \
    dictzip-win32 \


SUBDIRS += \
    libao \
    zstd \
    liblzo \
    OpenCC \
    ogg \
    vorbis \
    libhunspell \
    libebu \
    libgiflib \
    libpng \
    openjpeg \
    libjpeg \
    libwebp \
    libtiff \
    libleptonica \
#    jbig2enc \
    tesseract4

!greaterThan(QT_MAJOR_VERSION, 4) {
  win32:SUBDIRS += qt4-imageformats-wbmp/wbmp.pro \
    qt4-imageformats-webp/webp.pro
}
