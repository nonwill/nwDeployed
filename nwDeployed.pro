TEMPLATE = subdirs
CONFIG += ordered

win32:SUBDIRS = \
    zlib \
    libbzip2 \
    xz \
    libiconv \
    libxml2 \
    libxslt \
    sqlite3 \
    speex \
    libao 

SUBDIRS += \
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
