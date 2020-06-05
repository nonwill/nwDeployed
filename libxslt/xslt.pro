CONFIG -= qt

TEMPLATE = lib

TARGET = xslt

win32:system(cd win32 && cscript configure.js compiler=msvc xslt_debug=no mem_debug=no debugger=no debug=no)

win32:DEFINES += IN_LIBXSLT _WIN32

include($${PWD}/../nwDeployed.pri)

INCLUDEPATH += \
    $${PWD}/../libiconv/include \
    $${PWD}/../libxml2/include ./libxslt .


XSLT_INTDIR = $${PWD}/libxslt
SOURCES += \
    $${XSLT_INTDIR}/attributes.c \
    $${XSLT_INTDIR}/attrvt.c \
    $${XSLT_INTDIR}/documents.c \
    $${XSLT_INTDIR}/extensions.c \
    $${XSLT_INTDIR}/extra.c \
    $${XSLT_INTDIR}/functions.c \
    $${XSLT_INTDIR}/imports.c \
    $${XSLT_INTDIR}/keys.c \
    $${XSLT_INTDIR}/namespaces.c \
    $${XSLT_INTDIR}/numbers.c \
    $${XSLT_INTDIR}/pattern.c \
    $${XSLT_INTDIR}/preproc.c \
    $${XSLT_INTDIR}/security.c \
    $${XSLT_INTDIR}/templates.c \
    $${XSLT_INTDIR}/transform.c \
    $${XSLT_INTDIR}/variables.c \
    $${XSLT_INTDIR}/xslt.c \
    $${XSLT_INTDIR}/xsltlocale.c \
    $${XSLT_INTDIR}/xsltutils.c

#win32:DEF_FILE += wi32/libxslt.def

LIBS += -lxml2
