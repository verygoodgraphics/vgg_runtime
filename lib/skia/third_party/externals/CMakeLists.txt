######################################################################
# setup zlib
######################################################################
set(ZLIB_SRCS
  "zlib/adler32.c"
#"zlib/adler32_simd.c"
  "zlib/compress.c"
  "zlib/cpu_features.c"
  "zlib/crc32.c"
#"zlib/crc32_simd.c"
#"zlib/crc_folding.c"
  "zlib/deflate.c"
#"zlib/fill_window_sse.c"
  "zlib/gzclose.c"
  "zlib/gzlib.c"
  "zlib/gzread.c"
  "zlib/gzwrite.c"
  "zlib/infback.c"
  "zlib/inffast.c"
  "zlib/inflate.c"
  "zlib/inftrees.c"
  "zlib/trees.c"
  "zlib/uncompr.c"
  "zlib/zutil.c"
)
if (NOT EMSCRIPTEN)
  set(ZLIB_SRCS ${ZLIB_SRCS}
    "zlib/adler32_simd.c"
    "zlib/crc32_simd.c"
    "zlib/crc_folding.c"
    "zlib/fill_window_sse.c"
  )
endif()
add_library(zlib SHARED ${ZLIB_SRCS})

if (NOT EMSCRIPTEN)
    if(VGG_CPU_ARCH MATCHES "X86_64")
    set_target_properties(zlib PROPERTIES
      COMPILE_FLAGS "-msse3 -msse4.2 -mpclmul -DX86_NOT_WINDOWS -DADLER32_SIMD_SSSE3 -DCRC32_SIMD_SSE42_PCLMUL -DDEFLATE_FILL_WINDOW_SSE2"
    )
  endif()
endif()
target_include_directories(zlib PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/zlib
)

######################################################################
# setup libpng
######################################################################
set(LIBPNG_SRCS
  "libpng/png.c"
  "libpng/pngerror.c"
  "libpng/pngget.c"
  "libpng/pngmem.c"
  "libpng/pngpread.c"
  "libpng/pngread.c"
  "libpng/pngrio.c"
  "libpng/pngrtran.c"
  "libpng/pngrutil.c"
  "libpng/pngset.c"
  "libpng/pngtrans.c"
  "libpng/pngwio.c"
  "libpng/pngwrite.c"
  "libpng/pngwtran.c"
  "libpng/pngwutil.c"
)
if (NOT EMSCRIPTEN)
  set(LIBPNG_SRCS ${LIBPNG_SRCS}
    "libpng/intel/filter_sse2_intrinsics.c"
    "libpng/intel/intel_init.c"
  )
endif()
add_library(libpng SHARED ${LIBPNG_SRCS})
if(VGG_CPU_ARCH STREQUAL "ARM" AND VGG_PLATFORM STREQUAL "MACOS")
  # It seems that CMakeLists.txt in libpng directory is not used.
  # On Apple silicon, the optimization for arm must be off or result in link error.
  add_definitions(-DPNG_ARM_NEON_OPT=0)
endif()
if (NOT EMSCRIPTEN)
  # On arm64, it also could be compiled with PNG_INTEL_SSE.
  if(VGG_CPU_ARCH STREQUAL "X86_64")
  set_target_properties(libpng PROPERTIES
    COMPILE_FLAGS "-DPNG_INTEL_SSE"
  )
  endif()
endif()
add_custom_target(libpng_config
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt"
  "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h"
)
add_dependencies(libpng libpng_config)
target_include_directories(libpng PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/zlib
)
target_link_libraries(libpng zlib)

######################################################################
# setup freetype
######################################################################
set(FREETYPE_SRCS
  "freetype/src/autofit/autofit.c"
  "freetype/src/base/ftbase.c"
  "freetype/src/base/ftbbox.c"
  "freetype/src/base/ftbitmap.c"
  "freetype/src/base/ftdebug.c"
  "freetype/src/base/ftfstype.c"
  "freetype/src/base/ftgasp.c"
  "freetype/src/base/ftglyph.c"
  "freetype/src/base/ftinit.c"
  "freetype/src/base/ftmm.c"
  "freetype/src/base/ftpatent.c"
  "freetype/src/base/ftstroke.c"
  "freetype/src/base/ftsynth.c"
  "freetype/src/base/ftsystem.c"
  "freetype/src/base/fttype1.c"
  "freetype/src/base/ftwinfnt.c"
  "freetype/src/cff/cff.c"
  "freetype/src/cid/type1cid.c"
  "freetype/src/gzip/ftgzip.c"
  "freetype/src/psaux/psaux.c"
  "freetype/src/pshinter/pshinter.c"
  "freetype/src/psnames/psnames.c"
  "freetype/src/raster/raster.c"
  "freetype/src/sfnt/sfnt.c"
  "freetype/src/smooth/smooth.c"
  "freetype/src/truetype/truetype.c"
  "freetype/src/type1/type1.c"
)
add_library(freetype SHARED ${FREETYPE_SRCS})
target_include_directories(freetype PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libpng
  ${CMAKE_CURRENT_SOURCE_DIR}/../freetype2/
  ${CMAKE_CURRENT_SOURCE_DIR}/freetype/include
)
if (NOT EMSCRIPTEN)
  target_compile_definitions(freetype PRIVATE
    FT2_BUILD_LIBRARY
    FT_CONFIG_MODULES_H=<include/freetype-android/ftmodule.h>
    FT_CONFIG_OPTIONS_H=<include/freetype-android/ftoption.h>
  )
else()
  target_compile_definitions(freetype PRIVATE
    FT2_BUILD_LIBRARY
    FT_CONFIG_MODULES_H=<include/freetype-no-type1/ftmodule.h>
    FT_CONFIG_OPTIONS_H=<include/freetype-no-type1/ftoption.h>
  )
endif()
target_link_libraries(freetype libpng)

######################################################################
# setup libgifcodec
######################################################################
set(LIBGIFCODEC_SRCS
  "libgifcodec/SkGifImageReader.cpp"
  "libgifcodec/SkLibGifCodec.cpp"
)
add_library(libgifcodec STATIC ${LIBGIFCODEC_SRCS})
target_include_directories(libgifcodec PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../
)
set_property(TARGET libgifcodec PROPERTY POSITION_INDEPENDENT_CODE ON)

######################################################################
# setup libjpeg-turbo
######################################################################
set(LIBJPEG_TURBO_SRCS
  "libjpeg-turbo/jaricom.c"
  "libjpeg-turbo/jcapimin.c"
  "libjpeg-turbo/jcapistd.c"
  "libjpeg-turbo/jcarith.c"
  "libjpeg-turbo/jccoefct.c"
  "libjpeg-turbo/jccolor.c"
  "libjpeg-turbo/jcdctmgr.c"
  "libjpeg-turbo/jchuff.c"
  "libjpeg-turbo/jcinit.c"
  "libjpeg-turbo/jcmainct.c"
  "libjpeg-turbo/jcmarker.c"
  "libjpeg-turbo/jcmaster.c"
  "libjpeg-turbo/jcomapi.c"
  "libjpeg-turbo/jcparam.c"
  "libjpeg-turbo/jcphuff.c"
  "libjpeg-turbo/jcprepct.c"
  "libjpeg-turbo/jcsample.c"
  "libjpeg-turbo/jdapimin.c"
  "libjpeg-turbo/jdapistd.c"
  "libjpeg-turbo/jdarith.c"
  "libjpeg-turbo/jdcoefct.c"
  "libjpeg-turbo/jdcolor.c"
  "libjpeg-turbo/jddctmgr.c"
  "libjpeg-turbo/jdhuff.c"
  "libjpeg-turbo/jdinput.c"
  "libjpeg-turbo/jdmainct.c"
  "libjpeg-turbo/jdmarker.c"
  "libjpeg-turbo/jdmaster.c"
  "libjpeg-turbo/jdmerge.c"
  "libjpeg-turbo/jdphuff.c"
  "libjpeg-turbo/jdpostct.c"
  "libjpeg-turbo/jdsample.c"
  "libjpeg-turbo/jerror.c"
  "libjpeg-turbo/jfdctflt.c"
  "libjpeg-turbo/jfdctfst.c"
  "libjpeg-turbo/jfdctint.c"
  "libjpeg-turbo/jidctflt.c"
  "libjpeg-turbo/jidctfst.c"
  "libjpeg-turbo/jidctint.c"
  "libjpeg-turbo/jidctred.c"
  "libjpeg-turbo/jmemmgr.c"
  "libjpeg-turbo/jmemnobs.c"
  "libjpeg-turbo/jpeg_nbits_table.c"
  "libjpeg-turbo/jquant1.c"
  "libjpeg-turbo/jquant2.c"
  "libjpeg-turbo/jutils.c"
  "libjpeg-turbo/jsimd_none.c"
)
add_library(libjpeg-turbo SHARED ${LIBJPEG_TURBO_SRCS})
target_include_directories(libjpeg-turbo PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo
)
target_compile_definitions(libjpeg-turbo PRIVATE
  TURBO_FOR_WINDOWS
  C_ARITH_CODING_SUPPORTED=1
  D_ARITH_CODING_SUPPORTED=1
  USE_CLZ_INTRINSIC # for non-windows or clang
)

######################################################################
# setup libwebp
######################################################################
set(LIBWEBP_SRCS
  "libwebp/src/dec/alpha_dec.c"
  "libwebp/src/dec/buffer_dec.c"
  "libwebp/src/dec/frame_dec.c"
  "libwebp/src/dec/idec_dec.c"
  "libwebp/src/dec/io_dec.c"
  "libwebp/src/dec/quant_dec.c"
  "libwebp/src/dec/tree_dec.c"
  "libwebp/src/dec/vp8_dec.c"
  "libwebp/src/dec/vp8l_dec.c"
  "libwebp/src/dec/webp_dec.c"
  "libwebp/src/demux/anim_decode.c"
  "libwebp/src/demux/demux.c"
  "libwebp/src/dsp/alpha_processing.c"
  "libwebp/src/dsp/alpha_processing_mips_dsp_r2.c"
  "libwebp/src/dsp/alpha_processing_neon.c"
  "libwebp/src/dsp/alpha_processing_sse2.c"
  "libwebp/src/dsp/cost.c"
  "libwebp/src/dsp/cost_mips32.c"
  "libwebp/src/dsp/cost_mips_dsp_r2.c"
  "libwebp/src/dsp/cost_neon.c"
  "libwebp/src/dsp/cost_sse2.c"
  "libwebp/src/dsp/cpu.c"
  "libwebp/src/dsp/dec.c"
  "libwebp/src/dsp/dec_clip_tables.c"
  "libwebp/src/dsp/dec_mips32.c"
  "libwebp/src/dsp/dec_mips_dsp_r2.c"
  "libwebp/src/dsp/dec_msa.c"
  "libwebp/src/dsp/dec_neon.c"
  "libwebp/src/dsp/dec_sse2.c"
  "libwebp/src/dsp/enc.c"
  "libwebp/src/dsp/enc_mips32.c"
  "libwebp/src/dsp/enc_mips_dsp_r2.c"
  "libwebp/src/dsp/enc_msa.c"
  "libwebp/src/dsp/enc_neon.c"
  "libwebp/src/dsp/enc_sse2.c"
  "libwebp/src/dsp/filters.c"
  "libwebp/src/dsp/filters_mips_dsp_r2.c"
  "libwebp/src/dsp/filters_msa.c"
  "libwebp/src/dsp/filters_neon.c"
  "libwebp/src/dsp/filters_sse2.c"
  "libwebp/src/dsp/lossless.c"
  "libwebp/src/dsp/lossless_enc.c"
  "libwebp/src/dsp/lossless_enc_mips32.c"
  "libwebp/src/dsp/lossless_enc_mips_dsp_r2.c"
  "libwebp/src/dsp/lossless_enc_msa.c"
  "libwebp/src/dsp/lossless_enc_neon.c"
  "libwebp/src/dsp/lossless_enc_sse2.c"
  "libwebp/src/dsp/lossless_mips_dsp_r2.c"
  "libwebp/src/dsp/lossless_msa.c"
  "libwebp/src/dsp/lossless_neon.c"
  "libwebp/src/dsp/lossless_sse2.c"
  "libwebp/src/dsp/rescaler.c"
  "libwebp/src/dsp/rescaler_mips32.c"
  "libwebp/src/dsp/rescaler_mips_dsp_r2.c"
  "libwebp/src/dsp/rescaler_msa.c"
  "libwebp/src/dsp/rescaler_neon.c"
  "libwebp/src/dsp/rescaler_sse2.c"
  "libwebp/src/dsp/ssim.c"
  "libwebp/src/dsp/ssim_sse2.c"
  "libwebp/src/dsp/upsampling.c"
  "libwebp/src/dsp/upsampling_mips_dsp_r2.c"
  "libwebp/src/dsp/upsampling_msa.c"
  "libwebp/src/dsp/upsampling_neon.c"
  "libwebp/src/dsp/upsampling_sse2.c"
  "libwebp/src/dsp/yuv.c"
  "libwebp/src/dsp/yuv_mips32.c"
  "libwebp/src/dsp/yuv_mips_dsp_r2.c"
  "libwebp/src/dsp/yuv_neon.c"
  "libwebp/src/dsp/yuv_sse2.c"
  "libwebp/src/enc/alpha_enc.c"
  "libwebp/src/enc/analysis_enc.c"
  "libwebp/src/enc/backward_references_cost_enc.c"
  "libwebp/src/enc/backward_references_enc.c"
  "libwebp/src/enc/config_enc.c"
  "libwebp/src/enc/cost_enc.c"
  "libwebp/src/enc/filter_enc.c"
  "libwebp/src/enc/frame_enc.c"
  "libwebp/src/enc/histogram_enc.c"
  "libwebp/src/enc/iterator_enc.c"
  "libwebp/src/enc/near_lossless_enc.c"
  "libwebp/src/enc/picture_csp_enc.c"
  "libwebp/src/enc/picture_enc.c"
  "libwebp/src/enc/picture_psnr_enc.c"
  "libwebp/src/enc/picture_rescale_enc.c"
  "libwebp/src/enc/picture_tools_enc.c"
  "libwebp/src/enc/predictor_enc.c"
  "libwebp/src/enc/quant_enc.c"
  "libwebp/src/enc/syntax_enc.c"
  "libwebp/src/enc/token_enc.c"
  "libwebp/src/enc/tree_enc.c"
  "libwebp/src/enc/vp8l_enc.c"
  "libwebp/src/enc/webp_enc.c"
  "libwebp/src/mux/anim_encode.c"
  "libwebp/src/mux/muxedit.c"
  "libwebp/src/mux/muxinternal.c"
  "libwebp/src/mux/muxread.c"
  "libwebp/src/utils/bit_reader_utils.c"
  "libwebp/src/utils/bit_writer_utils.c"
  "libwebp/src/utils/color_cache_utils.c"
  "libwebp/src/utils/filters_utils.c"
  "libwebp/src/utils/huffman_encode_utils.c"
  "libwebp/src/utils/huffman_utils.c"
  "libwebp/src/utils/quant_levels_dec_utils.c"
  "libwebp/src/utils/quant_levels_utils.c"
  "libwebp/src/utils/random_utils.c"
  "libwebp/src/utils/rescaler_utils.c"
  "libwebp/src/utils/thread_utils.c"
  "libwebp/src/utils/utils.c"
)
if (NOT EMSCRIPTEN)
  set(LIBWEBP_SRCS ${LIBWEBP_SRCS}
    "libwebp/src/dsp/alpha_processing_sse41.c"
    "libwebp/src/dsp/dec_sse41.c"
    "libwebp/src/dsp/enc_sse41.c"
    "libwebp/src/dsp/lossless_enc_sse41.c"
    "libwebp/src/dsp/upsampling_sse41.c"
    "libwebp/src/dsp/yuv_sse41.c"
  )
endif()
add_library(libwebp SHARED ${LIBWEBP_SRCS})
if (NOT EMSCRIPTEN)
  set_target_properties(libpng PROPERTIES
    COMPILE_FLAGS "-msse4.1 -DWEBP_SWAP_16BIT_CSP"
  )
else()
  set_target_properties(libpng PROPERTIES
    COMPILE_FLAGS "-DWEBP_SWAP_16BIT_CSP"
  )
endif()
target_include_directories(libwebp PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/libwebp
)

# export include path
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/zlib
  ${CMAKE_CURRENT_SOURCE_DIR}/libpng
  ${CMAKE_CURRENT_SOURCE_DIR}/freetype/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libgifcodec
  ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo
  ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src
  PARENT_SCOPE
)
