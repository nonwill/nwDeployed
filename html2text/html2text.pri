
DEFINES += AUTO_PTR_BROKEN

include($${PWD}/../nwDeployed.pri)
win32:INCLUDEPATH += $${PWD} $${PWD}/../include/win32 \
    $${PWD}/../libiconv/include

SOURCES +=  $${PWD}/HTMLParser.tab.cc \
    $${PWD}/Area.cpp \
    $${PWD}/cmp_nocase.cpp \
    $${PWD}/format.cpp \
    $${PWD}/html.cpp \
    $${PWD}/html2text.cpp \
    $${PWD}/HTMLControl.cpp \
    $${PWD}/HTMLDriver.cpp \
    $${PWD}/Properties.cpp \
    $${PWD}/sgml.cpp \
    $${PWD}/table.cpp \
    $${PWD}/iconvstream.cpp

HEADERS += $${PWD}/HTMLParser.tab.hh \
    $${PWD}/Area.h \
    $${PWD}/cmp_nocase.h \
    $${PWD}/format.h \
    $${PWD}/html.h \
    $${PWD}/html2text.h \
    $${PWD}/HTMLControl.h \
    $${PWD}/HTMLDriver.h \
    $${PWD}/Properties.h \
    $${PWD}/sgml.h \
    $${PWD}/iconvstream.h

LIBS += -liconv
