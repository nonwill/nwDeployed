CONFIG -= qt

TEMPLATE = lib

TARGET = tesseract4
mac:CONFIG += static

DEFINES += TESS_EXPORTS

include($${PWD}/../nwDeployed.pri)

TESS_HEADERS = \
    src/api/apitypes.h \
    src/api/baseapi.h \
    src/api/capi.h \
    src/api/renderer.h \
    src/api/tess_version.h \
    src/ccmain/thresholder.h \
    src/ccmain/ltrresultiterator.h \
    src/ccmain/pageiterator.h \
    src/ccmain/resultiterator.h \
    src/ccmain/osdetect.h \
    src/ccstruct/publictypes.h \
    src/ccutil/genericvector.h \
    src/ccutil/helpers.h \
    src/ccutil/ocrclass.h \
    src/ccutil/platform.h \
    src/ccutil/serialis.h \
    src/ccutil/strngs.h \
    src/ccutil/unichar.h

HEADERS += $${TESS_HEADERS}

#  tesseract_src
SOURCES += \
    src/api/baseapi.cpp \
    src/api/capi.cpp \
    src/api/renderer.cpp \
    src/api/altorenderer.cpp \
    src/api/hocrrenderer.cpp \
    src/api/lstmboxrenderer.cpp \
    src/api/pdfrenderer.cpp \
    src/api/wordstrboxrenderer.cpp


win32:msvc*:CONFIG +=  AVX_OPT
win32:msvc*:CONFIG +=  AVX2_OPT
win32:msvc*:CONFIG +=  FMA_OPT
win32:msvc*:CONFIG +=  SSE41_OPT

contains(CONFIG,AVX_OPT) {
# APPEND arch_files_opt
DEFINES += __AVX__
SOURCES += src/arch/dotproductavx.cpp
}
contains(CONFIG,AVX2_OPT) {
# APPEND arch_files_opt
DEFINES += __AVX2__
SOURCES += src/arch/intsimdmatrixavx2.cpp
}
contains(CONFIG,FMA_OPT) {
# APPEND arch_files_opt
DEFINES += __FMA__
SOURCES += src/arch/dotproductfma.cpp
}
contains(CONFIG,SSE41_OPT) {
# APPEND arch_files_opt
DEFINES += __SSE4_1__
SOURCES += src/arch/dotproductsse.cpp
}

# GLOB tesseract_src
SOURCES += \
    src/ccmain/adaptions.cpp \
    src/ccmain/applybox.cpp \
    src/ccmain/control.cpp \
    src/ccmain/docqual.cpp \
    src/ccmain/equationdetect.cpp \
    src/ccmain/fixspace.cpp \
    src/ccmain/fixxht.cpp \
    src/ccmain/linerec.cpp \
    src/ccmain/ltrresultiterator.cpp \
    src/ccmain/mutableiterator.cpp \
    src/ccmain/osdetect.cpp \
    src/ccmain/output.cpp \
    src/ccmain/pageiterator.cpp \
    src/ccmain/pagesegmain.cpp \
    src/ccmain/pagewalk.cpp \
    src/ccmain/paragraphs.cpp \
    src/ccmain/paramsd.cpp \
    src/ccmain/par_control.cpp \
    src/ccmain/pgedit.cpp \
    src/ccmain/recogtraining.cpp \
    src/ccmain/reject.cpp \
    src/ccmain/resultiterator.cpp \
    src/ccmain/superscript.cpp \
    src/ccmain/tessbox.cpp \
    src/ccmain/tessedit.cpp \
    src/ccmain/tesseractclass.cpp \
    src/ccmain/tessvars.cpp \
    src/ccmain/tfacepp.cpp \
    src/ccmain/thresholder.cpp \
    src/ccmain/werdit.cpp

SOURCES += 	\
    src/ccstruct/blamer.cpp \
    src/ccstruct/blobbox.cpp \
    src/ccstruct/blobs.cpp \
    src/ccstruct/blread.cpp \
    src/ccstruct/boxread.cpp \
    src/ccstruct/boxword.cpp \
    src/ccstruct/ccstruct.cpp \
    src/ccstruct/coutln.cpp \
    src/ccstruct/detlinefit.cpp \
    src/ccstruct/dppoint.cpp \
    src/ccstruct/fontinfo.cpp \
    src/ccstruct/imagedata.cpp \
    src/ccstruct/linlsq.cpp \
    src/ccstruct/matrix.cpp \
    src/ccstruct/mod128.cpp \
    src/ccstruct/normalis.cpp \
    src/ccstruct/ocrblock.cpp \
    src/ccstruct/ocrpara.cpp \
    src/ccstruct/ocrrow.cpp \
    src/ccstruct/otsuthr.cpp \
    src/ccstruct/pageres.cpp \
    src/ccstruct/params_training_featdef.cpp \
    src/ccstruct/pdblock.cpp \
    src/ccstruct/points.cpp \
    src/ccstruct/polyaprx.cpp \
    src/ccstruct/polyblk.cpp \
    src/ccstruct/publictypes.cpp \
    src/ccstruct/quadlsq.cpp \
    src/ccstruct/quspline.cpp \
    src/ccstruct/ratngs.cpp \
    src/ccstruct/rect.cpp \
    src/ccstruct/rejctmap.cpp \
    src/ccstruct/seam.cpp \
    src/ccstruct/split.cpp \
    src/ccstruct/statistc.cpp \
    src/ccstruct/stepblob.cpp \
    src/ccstruct/werd.cpp

SOURCES += \
    src/ccutil/ambigs.cpp \
    src/ccutil/bitvector.cpp \
    src/ccutil/ccutil.cpp \
    src/ccutil/clst.cpp \
    src/ccutil/elst.cpp \
    src/ccutil/elst2.cpp \
    src/ccutil/errcode.cpp \
    src/ccutil/globaloc.cpp \
    src/ccutil/indexmapbidi.cpp \
    src/ccutil/mainblk.cpp \
    src/ccutil/params.cpp \
    src/ccutil/scanutils.cpp \
    src/ccutil/serialis.cpp \
    src/ccutil/strngs.cpp \
    src/ccutil/tessdatamanager.cpp \
    src/ccutil/tprintf.cpp \
    src/ccutil/unichar.cpp \
    src/ccutil/unicharcompress.cpp \
    src/ccutil/unicharmap.cpp \
    src/ccutil/unicharset.cpp \
    src/ccutil/unicodes.cpp \
    src/ccutil/universalambigs.cpp


SOURCES += \
    src/classify/adaptive.cpp \
    src/classify/adaptmatch.cpp \
    src/classify/blobclass.cpp \
    src/classify/classify.cpp \
    src/classify/cluster.cpp \
    src/classify/clusttool.cpp \
    src/classify/cutoffs.cpp \
    src/classify/featdefs.cpp \
    src/classify/float2int.cpp \
    src/classify/fpoint.cpp \
    src/classify/intfeaturedist.cpp \
    src/classify/intfeaturemap.cpp \
    src/classify/intfeaturespace.cpp \
    src/classify/intfx.cpp \
    src/classify/intmatcher.cpp \
    src/classify/intproto.cpp \
    src/classify/kdtree.cpp \
    src/classify/mf.cpp \
    src/classify/mfdefs.cpp \
    src/classify/mfoutline.cpp \
    src/classify/mfx.cpp \
    src/classify/normfeat.cpp \
    src/classify/normmatch.cpp \
    src/classify/ocrfeatures.cpp \
    src/classify/outfeat.cpp \
    src/classify/picofeat.cpp \
    src/classify/protos.cpp \
    src/classify/sampleiterator.cpp \
    src/classify/shapeclassifier.cpp \
    src/classify/shapetable.cpp \
    src/classify/tessclassifier.cpp \
    src/classify/trainingsample.cpp \
    src/classify/trainingsampleset.cpp

SOURCES += src/cutil/callcpp.cpp \
    src/cutil/cutil_class.cpp \
    src/cutil/emalloc.cpp \
    src/cutil/oldlist.cpp

SOURCES += \
    src/dict/context.cpp \
    src/dict/dawg.cpp \
    src/dict/dawg_cache.cpp \
    src/dict/dict.cpp \
    src/dict/hyphen.cpp \
    src/dict/permdawg.cpp \
    src/dict/stopper.cpp \
    src/dict/trie.cpp

SOURCES += \
    src/lstm/convolve.cpp \
    src/lstm/ctc.cpp \
    src/lstm/fullyconnected.cpp \
    src/lstm/functions.cpp \
    src/lstm/input.cpp \
    src/lstm/lstm.cpp \
    src/lstm/lstmrecognizer.cpp \
    src/lstm/lstmtrainer.cpp \
    src/lstm/maxpool.cpp \
    src/lstm/network.cpp \
    src/lstm/networkbuilder.cpp \
    src/lstm/networkio.cpp \
    src/lstm/parallel.cpp \
    src/lstm/plumbing.cpp \
    src/lstm/recodebeam.cpp \
    src/lstm/reconfig.cpp \
    src/lstm/reversed.cpp \
    src/lstm/series.cpp \
    src/lstm/stridemap.cpp \
    src/lstm/tfnetwork.cpp \
    src/lstm/weightmatrix.cpp

SOURCES += src/opencl/openclwrapper.cpp

SOURCES += \
    src/textord/alignedblob.cpp \
    src/textord/baselinedetect.cpp \
    src/textord/bbgrid.cpp \
    src/textord/blkocc.cpp \
    src/textord/blobgrid.cpp \
    src/textord/ccnontextdetect.cpp \
    src/textord/cjkpitch.cpp \
    src/textord/colfind.cpp \
    src/textord/colpartition.cpp \
    src/textord/colpartitiongrid.cpp \
    src/textord/colpartitionset.cpp \
    src/textord/devanagari_processing.cpp \
    src/textord/drawtord.cpp \
    src/textord/edgblob.cpp \
    src/textord/edgloop.cpp \
    src/textord/equationdetectbase.cpp \
    src/textord/fpchop.cpp \
    src/textord/gap_map.cpp \
    src/textord/imagefind.cpp \
    src/textord/linefind.cpp \
    src/textord/makerow.cpp \
    src/textord/oldbasel.cpp \
    src/textord/pithsync.cpp \
    src/textord/pitsync1.cpp \
    src/textord/scanedg.cpp \
    src/textord/sortflts.cpp \
    src/textord/strokewidth.cpp \
    src/textord/tabfind.cpp \
    src/textord/tablefind.cpp \
    src/textord/tablerecog.cpp \
    src/textord/tabvector.cpp \
    src/textord/textlineprojection.cpp \
    src/textord/textord.cpp \
    src/textord/topitch.cpp \
    src/textord/tordmain.cpp \
    src/textord/tospace.cpp \
    src/textord/tovars.cpp \
    src/textord/underlin.cpp \
    src/textord/wordseg.cpp \
    src/textord/workingpartset.cpp

SOURCES += \
    src/viewer/scrollview.cpp \
    src/viewer/svmnode.cpp \
#    src/viewer/svpaint.cpp \
    src/viewer/svutil.cpp

SOURCES += \
    src/wordrec/associate.cpp \
    src/wordrec/chop.cpp \
    src/wordrec/chopper.cpp \
    src/wordrec/drawfx.cpp \
    src/wordrec/findseam.cpp \
    src/wordrec/gradechop.cpp \
    src/wordrec/language_model.cpp \
    src/wordrec/lm_consistency.cpp \
    src/wordrec/lm_pain_points.cpp \
    src/wordrec/lm_state.cpp \
    src/wordrec/outlines.cpp \
    src/wordrec/params_model.cpp \
    src/wordrec/pieces.cpp \
    src/wordrec/plotedges.cpp \
    src/wordrec/render.cpp \
    src/wordrec/segsearch.cpp \
    src/wordrec/tface.cpp \
    src/wordrec/wordclass.cpp \
    src/wordrec/wordrec.cpp

# APPEND arch_files
SOURCES += \
    src/arch/dotproduct.cpp \
    src/arch/simddetect.cpp \
    src/arch/intsimdmatrix.cpp



INCLUDEPATH += \
    src/api \
    src/arch \
    src/ccmain \
    src/ccstruct \
    src/ccutil \
    src/classify \
    src/cutil \
    src/dict \
    src/lstm \
    src/opencl \
    src/textord \
    src/viewer \
    src/wordrec

win32:msvc* {
HEADERS += src/vs2010/tesseract/resource.h
RC_FILE += src/vs2010/tesseract/libtesseract.rc
}

INCLUDEPATH += $${PWD}/../libleptonica/src \

win32:LIBS += -lws2_32
LIBS += -llept
