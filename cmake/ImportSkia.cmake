# find_library(skia_DEB NAMES skia PATHS "/home/ysl/Code/skia/out/Static")
set(SKIA_INCLUDE_DIRS "/home/ysl/Code/skia/" "/home/ysl/Code/skia/include/")
set(SK_LIBS)

set(SKIA_LIB_NAMES
skia
# dng_sdk
# pathkit
# piex
# skcms
# skottie
# skparagraph
# skresources
# sksg
# skshaper
# sktext
# skunicode
# sksg
# svg
# wuffs
# png
# webp
# jpeg
# compression_utils_portable
# icu
# icu_bidi
)

set(SKIA_LIB_DIR "/home/ysl/Code/skia/out/Linux/Shared/Debug" CACHE STRING "" FORCE)

set(SKIA_COMPILE_DEFS 
SKSL_ENABLE_TRACING
SK_ENABLE_SPIRV_VALIDATION
SK_R32_SHIFT=16
SK_ENABLE_SKSL
SK_ENABLE_SKSL_IN_RASTER_PIPELINE
SK_ENABLE_PRECOMPILE
SK_GANESH
SK_USE_PERFETTO
SK_GAMMA_APPLY_TO_A8
SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
GR_TEST_UTILS=1
SK_TYPEFACE_FACTORY_FREETYPE
SK_GL
SK_ENABLE_DUMP_GPU
SK_SUPPORT_PDF
SK_CODEC_DECODES_JPEG
SK_CODEC_DECODES_JPEG_GAINMAPS
SK_XML
SK_ENABLE_ANDROID_UTILS
SK_HAS_HEIF_LIBRARY
SK_CODEC_DECODES_PNG
SK_CODEC_DECODES_RAW
SK_CODEC_DECODES_WEBP
SK_HAS_WUFFS_LIBRARY)


foreach(LIB ${SKIA_LIB_NAMES})
find_library(FOUND_LIB_${LIB} NAMES ${LIB} PATHS ${SKIA_LIB_DIR})
message(STATUS "${FOUND_LIB_${LIB}}" "  ${LIB}" " ${SKIA_LIB_DIR}")
list(APPEND SK_LIBS ${FOUND_LIB_${LIB}})
endforeach(LIB)

# macro(create_import_lib LIB_NAME SK_LIBS_OUT)
# find_library(skia_${LIB_NAME} NAMES ${LIB_NAME} PATHS "/home/ysl/Code/skia/out/Static")
# message(STATUS "skia library: " ${skia_${LIB_NAME}})
# # add_library(${LIB_NAME} STATIC IMPORTED)
# # set_target_properties(${LIB_NAME} PROPERTIES
# # 		IMPORTED_LOCATION "${skia_${LIB_NAME}}"
# # 		IMPORTED_LOCATION_DEBUG "${skia_${LIB_NAME}}"
# #   IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
# # )
# list(APPEND ${SK_LIBS_OUT} ${skia_${LIB_NAME}})
# endmacro()
