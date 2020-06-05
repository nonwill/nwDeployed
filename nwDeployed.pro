TEMPLATE = subdirs
CONFIG += ordered

win32:SUBDIRS = \
    zlib \
    libbzip2 \
    xz \
    zstd \
    libiconv \
    libxml2 \
    libxslt \
    sqlite3 \

SUBDIRS += \
    speex \
    libao \
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
