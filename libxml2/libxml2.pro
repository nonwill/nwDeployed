CONFIG -= qt

TEMPLATE = lib

TARGET = xml2

win32:system(cd win32 && cscript configure.js compiler=msvc xml_debug=no mem_debug=no run_debug=no debug=no)

win32:DEFINES += IN_LIBXML  _WIN32
include($${PWD}/../nwDeployed.pri)

INCLUDEPATH += $${PWD}/../libiconv/include \
    ./win32 ./include ./include/libxml

HEADERS += \
    ./win32/rcVersion.h \
    ./win32/rcVersion.h \


XML_INTDIR = $${PWD}
SOURCES += \
    $${XML_INTDIR}\buf.c \
    $${XML_INTDIR}\c14n.c \
    $${XML_INTDIR}\catalog.c \
    $${XML_INTDIR}\chvalid.c \
    $${XML_INTDIR}\debugXML.c \
    $${XML_INTDIR}\dict.c \
    $${XML_INTDIR}\DOCBparser.c \
    $${XML_INTDIR}\encoding.c \
    $${XML_INTDIR}\entities.c \
    $${XML_INTDIR}\error.c \
    $${XML_INTDIR}\globals.c \
    $${XML_INTDIR}\hash.c \
    $${XML_INTDIR}\HTMLparser.c \
    $${XML_INTDIR}\HTMLtree.c \
    $${XML_INTDIR}\legacy.c \
    $${XML_INTDIR}\list.c \
    $${XML_INTDIR}\nanoftp.c \
    $${XML_INTDIR}\nanohttp.c \
    $${XML_INTDIR}\parser.c \
    $${XML_INTDIR}\parserInternals.c \
    $${XML_INTDIR}\pattern.c \
    $${XML_INTDIR}\relaxng.c \
    $${XML_INTDIR}\SAX2.c \
    $${XML_INTDIR}\SAX.c \
    $${XML_INTDIR}\schematron.c \
    $${XML_INTDIR}\threads.c \
    $${XML_INTDIR}\tree.c \
    $${XML_INTDIR}\uri.c \
    $${XML_INTDIR}\valid.c \
    $${XML_INTDIR}\xinclude.c \
    $${XML_INTDIR}\xlink.c \
    $${XML_INTDIR}\xmlIO.c \
    $${XML_INTDIR}\xmlmemory.c \
    $${XML_INTDIR}\xmlreader.c \
    $${XML_INTDIR}\xmlregexp.c \
    $${XML_INTDIR}\xmlmodule.c \
    $${XML_INTDIR}\xmlsave.c \
    $${XML_INTDIR}\xmlschemas.c \
    $${XML_INTDIR}\xmlschemastypes.c \
    $${XML_INTDIR}\xmlunicode.c \
    $${XML_INTDIR}\xmlwriter.c \
    $${XML_INTDIR}\xpath.c \
    $${XML_INTDIR}\xpointer.c \
    $${XML_INTDIR}\xmlstring.c

win32:RC_FILE += win32/libxml2.rc

win32:LIBS += -lws2_32
LIBS += -liconv
