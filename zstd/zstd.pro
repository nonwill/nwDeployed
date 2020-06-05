CONFIG -= qt

TEMPLATE = lib

TARGET = zstd

CONFIG += static

DEFINES += ZSTD_MULTITHREAD=1 \
    ZSTD_LEGACY_SUPPORT=5 \
    XXH_PRIVATE_API
win32:DEFINES += ZSTD_HEAPMODE=0
#win32:DEFINES += ZSTD_DLL_EXPORT=1

include($${PWD}/../nwDeployed.pri)


HEADERS +=  \
    ./lib/common/pool.h \
    ./lib/common/threading.h \
    ./lib/common/bitstream.h \
    ./lib/common/error_private.h \
    ./lib/common/zstd_errors.h \
    ./lib/common/mem.h \
    ./lib/common/fse.h \
    ./lib/common/huf.h \
    ./lib/common/xxhash.h \
    ./lib/deprecated/zbuff.h \
    ./lib/legacy/zstd_legacy.h \
    ./lib/legacy/zstd_v01.h \
    ./lib/legacy/zstd_v02.h \
    ./lib/legacy/zstd_v03.h \
    ./lib/legacy/zstd_v04.h \
    ./lib/legacy/zstd_v05.h \
    ./lib/legacy/zstd_v06.h \
    ./lib/legacy/zstd_v07.h \
    ./lib/zstd.h \
    ./lib/common/zstd_internal.h \
    ./lib/compress/zstd_compress.h \
    ./lib/compress/zstd_compress_literals.h \
    ./lib/compress/zstd_compress_sequences.h \
    ./lib/compress/zstd_cwksp.h \
    ./lib/compress/zstd_compress_superblock.h \
    ./lib/compress/zstd_fast.h \
    ./lib/compress/zstd_double_fast.h \
    ./lib/compress/zstd_lazy.h \
    ./lib/compress/zstd_opt.h \
    ./lib/compress/zstd_ldm.h \
    ./lib/compress/zstdmt_compress.h \
    ./lib/decompress/zstd_ddict.h

SOURCES += \
    ./lib/common/pool.c \
    ./lib/common/debug.c \
    ./lib/common/threading.c \
    ./lib/common/entropy_common.c \
    ./lib/compress/hist.c \
    ./lib/common/error_private.c \
    ./lib/common/xxhash.c \
    ./lib/common/zstd_common.c \
    ./lib/common/fse_decompress.c \
    ./lib/compress/fse_compress.c \
    ./lib/compress/huf_compress.c \
    ./lib/compress/zstd_compress.c \
    ./lib/compress/zstd_compress_literals.c \
    ./lib/compress/zstd_compress_sequences.c \
    ./lib/compress/zstd_compress_superblock.c \
    ./lib/compress/zstd_fast.c \
    ./lib/compress/zstd_double_fast.c \
    ./lib/compress/zstd_lazy.c \
    ./lib/compress/zstd_opt.c \
    ./lib/compress/zstd_ldm.c \
    ./lib/compress/zstdmt_compress.c \
    ./lib/decompress/huf_decompress.c \
    ./lib/decompress/zstd_decompress.c \
    ./lib/decompress/zstd_decompress_block.c \
    ./lib/decompress/zstd_ddict.c \
    ./lib/deprecated/zbuff_common.c \
    ./lib/deprecated/zbuff_compress.c \
    ./lib/deprecated/zbuff_decompress.c \
    ./lib/dictBuilder/cover.c \
    ./lib/dictBuilder/fastcover.c \
    ./lib/dictBuilder/divsufsort.c \
    ./lib/dictBuilder/zdict.c \
    ./lib/legacy/zstd_v01.c \
    ./lib/legacy/zstd_v02.c \
    ./lib/legacy/zstd_v03.c \
    ./lib/legacy/zstd_v04.c \
    ./lib/legacy/zstd_v05.c \
    ./lib/legacy/zstd_v06.c \
    ./lib/legacy/zstd_v07.c

INCLUDEPATH += ./lib \
    ./programs/legacy \
    ./lib/legacy \
    ./lib/common \
    ./lib/dictBuilder

