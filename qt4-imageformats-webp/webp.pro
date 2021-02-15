TARGET  = qwebp

greaterThan(QT_MAJOR_VERSION, 4) {
    error($$TARGET is only an imageformat for Qt4!)
}

####
#include(../../qpluginbase.pri)
TEMPLATE = lib
VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
CONFIG += qt plugin
win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
##
#include(../qt_targets.pri)
QMAKE_TARGET_COMPANY = The Qt Company Ltd
QMAKE_TARGET_PRODUCT = Qt4
QMAKE_TARGET_DESCRIPTION = C++ application development framework.
QMAKE_TARGET_COPYRIGHT = Copyright (C) 2015 The Qt Company Ltd.
##
wince*:LIBS += $$QMAKE_LIBS_GUI

symbian: {
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = All -Tcb
    TARGET = $${TARGET}$${QT_LIBINFIX}
    load(armcc_warnings)

    # Make partial upgrade SIS file for Qt plugin dll's
    # Partial upgrade SIS file
    vendorinfo = \
        "; Localised Vendor name" \
        "%{\"Nokia\"}" \
        " " \
        "; Unique Vendor name" \
        ":\"Nokia, Qt\"" \
        " "
    isEmpty(QT_LIBINFIX): PARTIAL_UPGRADE_UID = 0x2001E61C
    else: PARTIAL_UPGRADE_UID = 0xE001E61C

    pu_header = "; Partial upgrade package for testing $${TARGET} changes without reinstalling everything" \
                "$${LITERAL_HASH}{\"$${TARGET}\"}, ($$PARTIAL_UPGRADE_UID), $${QT_MAJOR_VERSION},$${QT_MINOR_VERSION},$${QT_PATCH_VERSION}, TYPE=PU,RU"
    partial_upgrade.pkg_prerules = pu_header vendorinfo
    partial_upgrade.files = $$QMAKE_LIBDIR_QT/$${TARGET}.dll
    partial_upgrade.path = c:/sys/bin
    DEPLOYMENT += partial_upgrade
}
####

include(../nwDeployed.pri)
INCLUDEPATH += ../libwebp/src
LIBS += -lwebp

HEADERS += qwebphandler_p.h
SOURCES += main.cpp qwebphandler.cpp


QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/imageformats
target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
