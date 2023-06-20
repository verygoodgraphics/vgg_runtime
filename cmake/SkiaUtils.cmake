# Please refer to the skia build document for all supported featuers.
# Only those needed are covered here.
# https://skia.org/docs/user/build/
set(SKIA_SUPPORTED_FEATURES "elg;zlib;freetype;skparagraph;icu;gpu;vulkan;")
set(SKIA_PRESET_FEATURES_FOR_NATIVE
"skia_use_egl=true
skia_use_freetype=true
skia_use_zlib=true
skia_use_system_zlib=false
skia_canvaskit_enable_paragraph=true
skia_enable_skparagraph=true
skia_use_libwebp_encode=true
skia_use_icu=true
skia_enable_skunicode=true
skia_enable_gpu=true
skia_enable_fontmgr_custom_embedded=false
skia_canvaskit_enable_canvas_bindings=false
skia_canvaskit_enable_embedded_font=false
skia_use_vulkan=false 
skia_use_expat=false 
skia_use_piex=false 
skia_use_lua=false 
skia_use_dng_sdk=false 
skia_use_fontconfig=false 
skia_use_libheif=false 
skia_use_expat=false 
skia_use_vulkan=false 
skia_enable_pdf=false")

set(SKIA_PRESET_FEATURES_FOR_WASM
"skia_use_freetype=true 
skia_enable_svg=true 
skia_use_zlib=true 
skia_canvaskit_enable_paragraph=true 
skia_enable_skparagraph=true 
skia_use_libwebp_encode=true 
skia_use_icu=true 
skia_enable_skunicode=true 
skia_enable_gpu=true 
skia_use_expat=false 
skia_use_piex=false 
skia_use_lua=false 
skia_use_dng_sdk=false 
skia_use_fontconfig=false 
skia_use_libheif=false 
skia_use_expat=false 
skia_use_vulkan=false 
skia_enable_pdf=false")

function(list_from_json out_var json)
    set(list)
    string(JSON array ERROR_VARIABLE error GET "${json}" ${ARGN})
    if(NOT error)
        string(JSON len ERROR_VARIABLE error LENGTH "${array}")
        if(NOT error AND NOT len STREQUAL "0")
            math(EXPR last "${len} - 1")
            foreach(i RANGE "${last}")
                string(JSON item GET "${array}" "${i}")
                list(APPEND list "${item}")
            endforeach()
        endif()
    endif()
    set("${out_var}" "${list}" PARENT_SCOPE)
endfunction()

function(get_definitions out_var desc_json target)
    list_from_json(output "${desc_json}" "${target}" "defines")
    list(FILTER output INCLUDE REGEX "^SK_")
    set("${out_var}" "${output}" PARENT_SCOPE)
endfunction()

function(get_skia_gn_config out_options config platform link_type)

string(APPEND OPTIONS --args=)

# set target cpu for skia
if(${platform} IN_LIST VGG_WIN_TARGET_LIST OR ${platform} IN_LIST VGG_LINUX_TARGET_LIST)
  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_NATIVE})
  string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
elseif(platform STREQUAL "macOS-apple_silicon")
  string(APPEND OPTIONS "target_cpu='arm64'")
  string(APPEND OPTIONS ${SKIA_PRESET_FEATURES_FOR_NATIVE})
elseif(platform STREQUAL "WASM")
  string(APPEND OPTIONS "target_cpu='wasm'")
  string(APPEND OPTIONS ${SKIA_PRESET_FEATURES_FOR_WASM})
else()
  message(Fatal "target type for skia build is invalid: " ${platform})
endif()

if(NOT ${platform} IN_LIST VGG_WIN_TARGET_LIST)
# we assume non-window using gcc compatible compiler
if(config STREQUAL "RelWithDebInfo")
  string(APPEND OPTIONS " extra_cflags_cc=[\"-fvisibility=default\", \"-g\"]")
else()
  string(APPEND OPTIONS " extra_cflags_cc=[\"-fvisibility=default\"]")
endif()
elseif(platform STREQUAL "WASM")
string(APPEND OPTIONS " extra_cflags_cc=[\"-frtti\",\"-s\", \"-fvisibility=default\"] extra_cflags=[\"-Wno-unknown-warning-option\",\"-s\",\"-s\"]")
endif()

# set toolchain for skia
string(APPEND OPTIONS " cxx=\"${CMAKE_CXX_COMPILER}\"")
string(APPEND OPTIONS " cc=\"${CMAKE_C_COMPILER}\"")

# set config type for skia
if(config STREQUAL "Debug")
  string(APPEND OPTIONS " is_official_build=false is_debug=true")
elseif(config STREQUAL "Release")
  string(APPEND OPTIONS " is_official_build=true is_debug=false")
elseif(config STREQUAL "RelWithDebInfo")
  string(APPEND OPTIONS " is_official_build=true is_debug=false")
endif()

# set library link type for skia
if(link_type STREQUAL "dynamic")
  string(APPEND OPTIONS " is_component_build=true")
elseif(link_type STREQUAL "static")
  string(APPEND OPTIONS " is_compoeent_build=false")
endif()


# set features for skia
set("${out_options}" ${OPTIONS} PARENT_SCOPE)
endfunction()
