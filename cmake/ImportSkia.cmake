# find_library(skia_DEB NAMES skia PATHS "/home/ysl/Code/skia/out/Static")
set(SKIA_INCLUDE_DIRS "/home/ysl/Code/skia/" "/home/ysl/Code/skia/include/")
# message(STATUS "skia library: ", ${skia_DEB})
# add_library(skia::skia STATIC IMPORTED)
# set_target_properties(skia::skia PROPERTIES
#   IMPORTED_LOCATION "${skia_DEB}"
#   IMPORTED_LOCATION_DEBUG "${skia_DEB}"
#   IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
# )
set(SK_LIBS)

set(SKIA_LIB_NAMES
skia
dng_sdk
pathkit
piex
skcms
skottie
skparagraph
skresources
sksg
skshaper
sktext
skunicode
svg
wuffs
png
webp
jpeg
compression_utils_portable
icu
icu_bidi
)

set(SKIA_LIB_DIR "/home/ysl/Code/skia/out/Static")

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
