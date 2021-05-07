
greaterThan(QT_MAJOR_VERSION, 4) {
    #unix:QMAKE_LFLAGS += -fuse-ld=gold
    TARGET_ARCH=$${QT_ARCH}
} else {
    #linux:QMAKE_LFLAGS += -fuse-ld=bfd
    TARGET_ARCH=$${QMAKE_TARGET.arch}
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
} else {
  linux {
    BSDK_TYPE = linux
  } else {
    mac {
      BSDK_TYPE = mac
    } else {
      BSDK_TYPE = unix
    }
  }
}

NWQT_PVER_FIX=$${QT_MAJOR_VERSION}_$${ARCHITECTURE}
isEmpty(CUR_PRJ_DIR):CUR_PRJ_DIR = $$PWD

NWDEP_LIB = $${CUR_PRJ_DIR}/lib$${NWQT_PVER_FIX}_$${BSDK_TYPE}
NWDEP_BIN = $${CUR_PRJ_DIR}/bin$${NWQT_PVER_FIX}_$${BSDK_TYPE}
contains(TEMPLATE,lib) {
    unix:!contains(CONFIG,static) {
        CONFIG += plugin
    }
    DESTDIR = $${NWDEP_LIB}
    DLLDESTDIR = $${NWDEP_BIN}
} else {
    DESTDIR = $${NWDEP_BIN}
}

LIBS += -L$${NWDEP_LIB}

win32 {
    win32-msvc* {
        contains(TARGET_ARCH, x86_64) {
            DEFINES += __WIN64
        } else {
            QMAKE_LFLAGS += /LARGEADDRESSAWARE
        }
    } else {
        !x64:QMAKE_LFLAGS += -Wl,--large-address-aware
    }
}
