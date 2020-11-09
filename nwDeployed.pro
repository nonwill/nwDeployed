TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    zlib \
    libbzip2 \
    xz \
    speex \
    mimetex

win32:SUBDIRS += \
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

