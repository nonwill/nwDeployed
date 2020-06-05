CONFIG -= qt

TEMPLATE = lib

TARGET = lept

mac:CONFIG += static


win32::msvc*:DEFINES += COMPILER_MSVC
DEFINES += LIBLEPT_EXPORTS HAVE_CONFIG_H NO_CONSOLE_IO
DEFINES += OPJ_STATIC
DEFINES -= UNICODE _UNICODE

win32::msvc*:QMAKE_CFLAGS += /TP # Compile C as CPlusPlus, this is important.

include($${PWD}/../nwDeployed.pri)

HEADERS +=  \
    endianness.h \
    config_auto.h \
    src/allheaders.h \
    src/alltypes.h \
    src/array.h \
    src/arrayaccess.h \
    src/bbuffer.h \
    src/bilateral.h \
    src/bmf.h \
    src/bmfdata.h \
    src/bmp.h \
    src/ccbord.h \
    src/colorinfo.h \
    src/dewarp.h \
    src/endianness.h \
    src/environ.h \
    src/gplot.h \
    src/heap.h \
    src/imageio.h \
    src/jbclass.h \
    src/leptwin.h \
    src/list.h \
    src/morph.h \
    src/pix.h \
    src/ptra.h \
    src/queue.h \
    src/rbtree.h \
    src/readbarcode.h \
    src/recog.h \
    src/regutils.h \
    src/stack.h \
    src/stringcode.h \
    src/sudoku.h \
    src/watershed.h


SOURCES += \
    src/adaptmap.c \
    src/affine.c \
    src/affinecompose.c \
    src/arrayaccess.c \
    src/bardecode.c \
    src/baseline.c \
    src/bbuffer.c \
    src/bilateral.c \
    src/bilinear.c \
    src/binarize.c \
    src/binexpand.c \
    src/binreduce.c \
    src/blend.c \
    src/bmf.c \
    src/bmpio.c \
    src/bmpiostub.c \
    src/bootnumgen1.c \
    src/bootnumgen2.c \
    src/bootnumgen3.c \
    src/bootnumgen4.c \
    src/boxbasic.c \
    src/boxfunc1.c \
    src/boxfunc2.c \
    src/boxfunc3.c \
    src/boxfunc4.c \
    src/boxfunc5.c \
    src/bytearray.c \
    src/ccbord.c \
    src/ccthin.c \
    src/checkerboard.c \
    src/classapp.c \
    src/colorcontent.c \
    src/colorinfo.c \
    src/coloring.c \
    src/colormap.c \
    src/colormorph.c \
    src/colorquant1.c \
    src/colorquant2.c \
    src/colorseg.c \
    src/colorspace.c \
    src/compare.c \
    src/conncomp.c \
    src/convertfiles.c \
    src/convolve.c \
    src/correlscore.c \
    src/dewarp1.c \
    src/dewarp2.c \
    src/dewarp3.c \
    src/dewarp4.c \
    src/dnabasic.c \
    src/dnafunc1.c \
    src/dnahash.c \
    src/dwacomb.2.c \
    src/dwacomblow.2.c \
    src/edge.c \
    src/encoding.c \
    src/enhance.c \
    src/fhmtauto.c \
    src/fhmtgen.1.c \
    src/fhmtgenlow.1.c \
    src/finditalic.c \
    src/flipdetect.c \
    src/fliphmtgen.c \
    src/fmorphauto.c \
    src/fmorphgen.1.c \
    src/fmorphgenlow.1.c \
    src/fpix1.c \
    src/fpix2.c \
    src/gifio.c \
    src/gifiostub.c \
    src/gplot.c \
    src/graphics.c \
    src/graymorph.c \
    src/grayquant.c \
    src/heap.c \
    src/jbclass.c \
    src/jp2kheader.c \
    src/jp2kheaderstub.c \
    src/jp2kio.c \
    src/jp2kiostub.c \
    src/jpegio.c \
    src/jpegiostub.c \
    src/kernel.c \
    src/leptwin.c \
    src/libversions.c \
    src/list.c \
    src/map.c \
    src/maze.c \
    src/morph.c \
    src/morphapp.c \
    src/morphdwa.c \
    src/morphseq.c \
    src/numabasic.c \
    src/numafunc1.c \
    src/numafunc2.c \
    src/pageseg.c \
    src/paintcmap.c \
    src/parseprotos.c \
    src/partify.c \
    src/partition.c \
    src/pdfio1.c \
    src/pdfio1stub.c \
    src/pdfio2.c \
    src/pdfio2stub.c \
    src/pix1.c \
    src/pix2.c \
    src/pix3.c \
    src/pix4.c \
    src/pix5.c \
    src/pixabasic.c \
    src/pixacc.c \
    src/pixafunc1.c \
    src/pixafunc2.c \
    src/pixalloc.c \
    src/pixarith.c \
    src/pixcomp.c \
    src/pixconv.c \
    src/pixlabel.c \
    src/pixtiling.c \
    src/pngio.c \
    src/pngiostub.c \
    src/pnmio.c \
    src/pnmiostub.c \
    src/projective.c \
    src/psio1.c \
    src/psio1stub.c \
    src/psio2.c \
    src/psio2stub.c \
    src/ptabasic.c \
    src/ptafunc1.c \
    src/ptafunc2.c \
    src/ptra.c \
    src/quadtree.c \
    src/queue.c \
    src/rank.c \
    src/rbtree.c \
    src/readbarcode.c \
    src/readfile.c \
    src/recogbasic.c \
    src/recogdid.c \
    src/recogident.c \
    src/recogtrain.c \
    src/regutils.c \
    src/rop.c \
    src/roplow.c \
    src/rotate.c \
    src/rotateam.c \
    src/rotateorth.c \
    src/rotateshear.c \
    src/runlength.c \
    src/sarray1.c \
    src/sarray2.c \
    src/scale1.c \
    src/scale2.c \
    src/seedfill.c \
    src/sel1.c \
    src/sel2.c \
    src/selgen.c \
    src/shear.c \
    src/skew.c \
    src/spixio.c \
    src/stack.c \
    src/stringcode.c \
    src/strokes.c \
    src/sudoku.c \
    src/textops.c \
    src/tiffio.c \
    src/tiffiostub.c \
    src/utils1.c \
    src/utils2.c \
    src/warper.c \
    src/watershed.c \
    src/webpanimio.c \
    src/webpanimiostub.c \
    src/webpio.c \
    src/webpiostub.c \
    src/writefile.c \
    src/zlibmem.c \
    src/zlibmemstub.c


INCLUDEPATH += . $${PWD}/src \
    $${PWD}/../openjpeg \
    $${PWD}/../libwebp/src \
    $${PWD}/../libjpeg \
    $${PWD}/../libpng \
    $${PWD}/../libgiflib \
    $${PWD}/../libtiff

win32:LIBS += -lGdi32 -lUser32

LIBS += -lgiflib -lpng -ljpeg \
    -ltiff -lwebp -lopenjpeg -lz


INCLUDEPATH += ../zlib/include
