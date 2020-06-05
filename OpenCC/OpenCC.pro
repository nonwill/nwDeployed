#-------------------------------------------------
#
# Project created by QtCreator 2018-11-20T09:11:47
#
#-------------------------------------------------

QT       -= qt
TEMPLATE = lib

mac {
TARGET = opencc.2
} else {
TARGET = opencc
}

mac:CONFIG += static

win32:DEFINES += OPENCC_EXPORT=__declspec\(dllexport\)

include($${PWD}/../nwDeployed.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += ./src . deps/darts-clone deps/rapidjson-0.11 deps/tclap-1.2.1

HEADERS += src/Opencc_Export.h\
    src/BinaryDict.hpp \
    src/Common.hpp \
    src/Config.hpp \
    src/Conversion.hpp \
    src/ConversionChain.hpp \
    src/Converter.hpp \
    src/DartsDict.hpp \
    src/Dict.hpp \
    src/DictConverter.hpp \
    src/DictEntry.hpp \
    src/DictGroup.hpp \
    src/Exception.hpp \
    src/Export.hpp \
    src/Lexicon.hpp \
    src/MaxMatchSegmentation.hpp \
    src/Optional.hpp \
    src/PhraseExtract.hpp \
    src/Segmentation.hpp \
    src/Segments.hpp \
    src/SerializableDict.hpp \
    src/SimpleConverter.hpp \
    src/TextDict.hpp \
    src/UTF8StringSlice.hpp \
    src/UTF8Util.hpp \
    src/opencc.h

SOURCES += \
    src/BinaryDict.cpp \
    src/Config.cpp \
    src/Conversion.cpp \
    src/ConversionChain.cpp \
    src/Converter.cpp \
    src/DartsDict.cpp \
    src/Dict.cpp \
    src/DictConverter.cpp \
    src/DictEntry.cpp \
    src/DictGroup.cpp \
    src/MaxMatchSegmentation.cpp \
    src/PhraseExtract.cpp \
    src/SimpleConverter.cpp \
    src/Segmentation.cpp \
    src/TextDict.cpp \
    src/UTF8StringSlice.cpp \
    src/UTF8Util.cpp \

win32:LIBS += -lksuser -lwinmm

win32:DEF_FILE =

