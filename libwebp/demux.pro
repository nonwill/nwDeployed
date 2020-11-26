CONFIG -= qt warn_on
CONFIG += exceptions_off rtti_off warn_off

TEMPLATE = lib

TARGET = webpdemux

unix:CONFIG += static
win32:DEFINES += WEBP_EXTERN=__declspec\(dllexport\)

DEFINES += WEBP_USE_THREAD
win32:msvc*:DEFINES += __SSE2__ __SSE4_1__

include($${PWD}/../nwDeployed.pri)

SOURCES += \
    src/demux/demux.c \
    src/demux/anim_decode.c

HEADERS +=  \
    src/webp/decode.h \
    src/webp/demux.h \
    src/webp/mux_types.h \
    src/webp/format_constants.h \
    src/webp/types.h

integrity {
    QMAKE_CFLAGS += -c99
}

INCLUDEPATH += . src src/webp

LIBS += -lwebp
