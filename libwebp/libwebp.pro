CONFIG -= qt warn_on
CONFIG += exceptions_off rtti_off warn_off

TEMPLATE = lib

TARGET = webp

unix:CONFIG += static
win32:msvc*:DEFINES += WEBP_EXTERN=__declspec\(dllexport\)

DEFINES += WEBP_USE_THREAD
win32:msvc*:DEFINES += __SSE2__ __SSE4_1__

include($${PWD}/../nwDeployed.pri)

SOURCES += \
    src/dec/alpha_dec.c \
    src/dec/buffer_dec.c \
    src/dec/frame_dec.c \
    src/dec/idec_dec.c \
    src/dec/io_dec.c \
    src/dec/quant_dec.c \
    src/dec/tree_dec.c \
    src/dec/vp8_dec.c \
    src/dec/vp8l_dec.c \
    src/dec/webp_dec.c \
    src/demux/demux.c \
    src/demux/anim_decode.c \
    src/dsp/alpha_processing.c \
    src/dsp/alpha_processing_mips_dsp_r2.c \
    src/dsp/alpha_processing_sse2.c \
    src/dsp/alpha_processing_sse41.c \
    src/dsp/cost.c \
    src/dsp/cost_mips32.c \
    src/dsp/cost_mips_dsp_r2.c \
    src/dsp/cost_sse2.c \
    src/dsp/cpu.c \
    src/dsp/dec.c \
    src/dsp/dec_clip_tables.c \
    src/dsp/dec_mips32.c \
    src/dsp/dec_mips_dsp_r2.c \
    src/dsp/dec_msa.c \
    src/dsp/dec_sse2.c \
    src/dsp/dec_sse41.c \
    src/dsp/enc.c \
    src/dsp/enc_mips32.c \
    src/dsp/enc_mips_dsp_r2.c \
    src/dsp/enc_msa.c \
    src/dsp/enc_sse2.c \
    src/dsp/enc_sse41.c \
    src/dsp/filters.c \
    src/dsp/filters_mips_dsp_r2.c \
    src/dsp/filters_msa.c \
    src/dsp/filters_sse2.c \
    src/dsp/lossless.c \
    src/dsp/lossless_enc.c \
    src/dsp/lossless_enc_mips32.c \
    src/dsp/lossless_enc_mips_dsp_r2.c \
    src/dsp/lossless_enc_msa.c \
    src/dsp/lossless_enc_sse2.c \
    src/dsp/lossless_enc_sse41.c \
    src/dsp/lossless_mips_dsp_r2.c \
    src/dsp/rescaler.c \
    src/dsp/rescaler_mips32.c \
    src/dsp/rescaler_mips_dsp_r2.c \
    src/dsp/rescaler_msa.c \
    src/dsp/rescaler_sse2.c \
    src/dsp/ssim.c \
    src/dsp/ssim_sse2.c \
    src/dsp/upsampling.c \
    src/dsp/upsampling_mips_dsp_r2.c \
    src/dsp/upsampling_msa.c \
    src/dsp/upsampling_sse2.c \
    src/dsp/upsampling_sse41.c \
    src/dsp/yuv.c \
    src/dsp/yuv_mips_dsp_r2.c \
    src/dsp/lossless_sse2.c \
    src/dsp/yuv_mips32.c \
    src/dsp/yuv_sse2.c \
    src/dsp/yuv_sse41.c \
    src/enc/alpha_enc.c \
    src/enc/analysis_enc.c \
    src/enc/backward_references_cost_enc.c \
    src/enc/backward_references_enc.c \
    src/enc/config_enc.c \
    src/enc/cost_enc.c \
    src/enc/filter_enc.c \
    src/enc/frame_enc.c \
    src/enc/histogram_enc.c \
    src/enc/iterator_enc.c \
    src/enc/near_lossless_enc.c \
    src/enc/picture_enc.c \
    src/enc/picture_csp_enc.c \
    src/enc/picture_psnr_enc.c \
    src/enc/picture_rescale_enc.c \
    src/enc/picture_tools_enc.c \
    src/enc/predictor_enc.c \
    src/enc/quant_enc.c \
    src/enc/syntax_enc.c \
    src/enc/token_enc.c \
    src/enc/tree_enc.c \
    src/enc/vp8l_enc.c \
    src/enc/webp_enc.c \
    src/mux/anim_encode.c \
    src/mux/muxedit.c \
    src/mux/muxinternal.c \
    src/mux/muxread.c \
    src/utils/bit_reader_utils.c \
    src/utils/bit_writer_utils.c \
    src/utils/color_cache_utils.c \
    src/utils/filters_utils.c \
    src/utils/huffman_utils.c \
    src/utils/huffman_encode_utils.c \
    src/utils/quant_levels_utils.c \
    src/utils/quant_levels_dec_utils.c \
    src/utils/random_utils.c \
    src/utils/rescaler_utils.c \
    src/utils/thread_utils.c \
    src/utils/utils.c

HEADERS +=  \
    src/dec/alphai_dec.h \
    src/dec/common_dec.h \
    src/dec/vp8_dec.h \
    src/dec/vp8i_dec.h \
    src/dec/vp8li_dec.h \
    src/dec/webpi_dec.h \
    src/dsp/common_sse2.h \
    src/dsp/common_sse41.h \
    src/dsp/dsp.h \
    src/dsp/lossless_common.h \
    src/dsp/lossless.h \
    src/dsp/mips_macro.h \
    src/dsp/msa_macro.h \
    src/dsp/neon.h \
    src/dsp/quant.h \
    src/dsp/yuv.h \
    src/enc/backward_references_enc.h \
    src/enc/cost_enc.h \
    src/enc/histogram_enc.h \
    src/enc/vp8i_enc.h \
    src/enc/vp8li_enc.h \
    src/mux/animi.h \
    src/mux/muxi.h \
    src/utils/bit_reader_inl_utils.h \
    src/utils/bit_reader_utils.h \
    src/utils/bit_writer_utils.h \
    src/utils/color_cache_utils.h \
    src/utils/endian_inl_utils.h \
    src/utils/filters_utils.h \
    src/utils/huffman_encode_utils.h \
    src/utils/huffman_utils.h \
    src/utils/quant_levels_dec_utils.h \
    src/utils/quant_levels_utils.h \
    src/utils/random_utils.h \
    src/utils/rescaler_utils.h \
    src/utils/thread_utils.h \
    src/utils/utils.h \
    src/webp/decode.h \
    src/webp/demux.h \
    src/webp/encode.h \
    src/webp/format_constants.h \
    src/webp/mux.h \
    src/webp/mux_types.h \
    src/webp/types.h

integrity {
    QMAKE_CFLAGS += -c99
}

equals(QT_ARCH, arm)|equals(QT_ARCH, arm64) {
    SOURCES_FOR_NEON += \
        src/dsp/alpha_processing_neon.c \
        src/dsp/cost_neon.c \
        src/dsp/dec_neon.c \
        src/dsp/enc_neon.c \
        src/dsp/filters_neon.c \
        src/dsp/lossless_enc_neon.c \
        src/dsp/lossless_neon.c \
        src/dsp/rescaler_neon.c \
        src/dsp/upsampling_neon.c \
        src/dsp/yuv_neon.c

    contains(QT_CPU_FEATURES.$$QT_ARCH, neon) {
        # Default compiler settings include this feature, so just add to SOURCES
        SOURCES += $$SOURCES_FOR_NEON
    } else {
        neon_comp.commands = $$QMAKE_CC -c $(CFLAGS)
        neon_comp.commands += $$QMAKE_CFLAGS_NEON
        neon_comp.commands += $(INCPATH) ${QMAKE_FILE_IN}
        msvc: neon_comp.commands += -Fo${QMAKE_FILE_OUT}
        else: neon_comp.commands += -o ${QMAKE_FILE_OUT}
        neon_comp.dependency_type = TYPE_C
        neon_comp.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_OBJ)}
        neon_comp.input = SOURCES_FOR_NEON
        neon_comp.variable_out = OBJECTS
        neon_comp.name = compiling[neon] ${QMAKE_FILE_IN}
        silent: neon_comp.commands = @echo compiling[neon] ${QMAKE_FILE_IN} && $$neon_comp.commands
        QMAKE_EXTRA_COMPILERS += neon_comp
    }
}

android:!android-embedded {
    SOURCES += $${NDK_ROOT}/sources/android/cpufeatures/cpu-features.c
    INCLUDEPATH += $${NDK_ROOT}/sources/android/cpufeatures
}

INCLUDEPATH += .

LIBS +=
