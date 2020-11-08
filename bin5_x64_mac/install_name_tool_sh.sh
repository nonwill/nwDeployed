#/bin/sh
otool -L GoldenDict.app/Contents/MacOS/GoldenDict

install_name_tool -change libGDCore.1.dylib "@rpath/libGDCore.1.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change libao.1.dylib "@rpath/libao.1.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change libtesseract4.1.dylib "@rpath/libtesseract4.1.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change liblept.1.dylib "@rpath/liblept.1.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change libmacosx.1.dylib "@rpath/libmacosx.1.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change libocrscreen.1.dylib "@rpath/libocrscreen.1.dylib" GoldenDict.app/Contents/MacOS/GoldenDict

install_name_tool -change libavcodec-gd.58.dylib "@rpath/libavcodec-gd.58.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change libavformat-gd.58.dylib "@rpath/libavformat-gd.58.dylib" GoldenDict.app/Contents/MacOS/GoldenDict
install_name_tool -change libavutil-gd.56.dylib "@rpath/libavutil-gd.56.dylib" GoldenDict.app/Contents/MacOS/GoldenDict

