
greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}
contains(TARGET_ARCH, x86_64) {
    ARCHITECTURE = x64
} else {
    ARCHITECTURE = x86
}

win32 {
  win32-msvc* {
    BSDK_TYPE = msvc
  } else {
    BSDK_TYPE = mingw
  }
}
unix:mac:BSDK_TYPE = mac

NWQT_PVER_FIX=$${QT_MAJOR_VERSION}_$${ARCHITECTURE}
isEmpty(CUR_PRJ_DIR):CUR_PRJ_DIR = $$PWD

NWDEP_LIB = $${CUR_PRJ_DIR}/lib$${NWQT_PVER_FIX}_$${BSDK_TYPE}
NWDEP_BIN = $${CUR_PRJ_DIR}/bin$${NWQT_PVER_FIX}_$${BSDK_TYPE}
contains(TEMPLATE,lib) {
    DESTDIR = $${NWDEP_LIB}
    DLLDESTDIR = $${NWDEP_BIN}
} else {
    DESTDIR = $${NWDEP_BIN}
}

LIBS += -L$${NWDEP_LIB}

