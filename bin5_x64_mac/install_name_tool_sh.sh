#/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

install_name_tool -change libGDCore.dylib "@rpath/libGDCore.dylib" $DIR/GoldenDict.app/Contents/MacOS/GoldenDict
#install_name_tool -change libao.dylib "@rpath/libao.dylib" GoldenDict.app/Contents/MacOS/GoldenDict

install_name_tool -change libavcodec-gd.58.dylib "@rpath/libavcodec-gd.58.dylib" $DIR/GoldenDict.app/Contents/Frameworks/libavformat-gd.58.dylib
install_name_tool -change libavutil-gd.56.dylib "@rpath/libavutil-gd.56.dylib" $DIR/GoldenDict.app/Contents/Frameworks/libavformat-gd.58.dylib
install_name_tool -change libavutil-gd.56.dylib "@rpath/libavutil-gd.56.dylib" $DIR/GoldenDict.app/Contents/Frameworks/libavcodec-gd.58.dylib

install_name_tool -change libavcodec-gd.58.dylib "@rpath/libavcodec-gd.58.dylib" $DIR/GoldenDict.app/Contents/MacOS/gdp/libgdp.adp.ffmpeg.dylib
install_name_tool -change libavformat-gd.58.dylib "@rpath/libavformat-gd.58.dylib" $DIR/GoldenDict.app/Contents/MacOS/gdp/libgdp.adp.ffmpeg.dylib
install_name_tool -change libavutil-gd.56.dylib "@rpath/libavutil-gd.56.dylib" $DIR/GoldenDict.app/Contents/MacOS/gdp/libgdp.adp.ffmpeg.dylib

install_name_tool -change libGDCore.dylib "@rpath/libGDCore.dylib" $DIR/GoldenDict.app/Contents/MacOS/gdp/libgdp.adp.ffmpeg.dylib
install_name_tool -change libGDCore.dylib "@rpath/libGDCore.dylib" $DIR/GoldenDict.app/Contents/MacOS/gdp/libgdp.gsc.fromcliboard.dylib
install_name_tool -change libGDCore.dylib "@rpath/libGDCore.dylib" $DIR/GoldenDict.app/Contents/MacOS/gdp/libgdp.ocr.tesseract.dylib

rm -f $DIR/GoldenDict.dmg
~/Qt5.9.9/5.9.9/clang_64/bin/macdeployqt $DIR/GoldenDict.app -dmg
