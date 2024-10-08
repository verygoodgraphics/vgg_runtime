# Please refer to the skia build document for all supported featuers.
# Only those needed are covered here.
# https://skia.org/docs/user/build/
if(NOT VGG_CONTAINER_FOR_QT 
  AND NOT VGG_VAR_TARGET MATCHES "^iOS"
  AND NOT VGG_VAR_TARGET MATCHES "^Android")
  find_package(Vulkan)
endif()

if (VGG_VAR_TARGET MATCHES "^Android")
  string(REPLACE "android-" "" api_level ${ANDROID_PLATFORM})
  if(NOT api_level LESS 24)
    message(STATUS "Found Android API Level ${api_level}, vulkan enabled.")
    set(VULKAN_AVAILABLE "true")
  else()
    message(STATUS "Found Android API Level ${api_level}, vulkan only support API_LEVEL>=24, disabled")
    set(VULKAN_AVAILABLE "false")
  endif()
else()
  if(Vulkan_FOUND)
    set(VULKAN_AVAILABLE "true")
  else()
    set(VULKAN_AVAILABLE "false")
  endif()
endif()

set(SKIA_PRESET_FEATURES_FOR_LINUX
"skia_use_egl=true
skia_use_freetype=true
skia_use_system_freetype2=false
skia_use_system_libjpeg_turbo=false
skia_use_jpeg_gainmaps=true
skia_use_libjpeg_turbo_decode=true
skia_use_libjpeg_turbo_encode=true
skia_use_system_icu=false
skia_use_system_libwebp=false
skia_use_system_libpng=false
skia_use_system_harfbuzz=false
skia_use_zlib=true
skia_use_system_zlib=false
skia_canvaskit_enable_paragraph=true
skia_enable_skparagraph=true
skia_use_libwebp_encode=true
skia_use_icu=true
skia_use_client_icu=true
skia_enable_skunicode=true
skia_enable_gpu=true
skia_enable_fontmgr_custom_embedded=false
skia_canvaskit_enable_canvas_bindings=false
skia_canvaskit_enable_embedded_font=false
skia_enable_ganesh=true
skia_use_vulkan=${VULKAN_AVAILABLE}
skia_use_piex=false
skia_use_lua=false
skia_use_dng_sdk=false
skia_use_fontconfig=false
skia_use_libheif=false
skia_use_system_expat=false
skia_use_expat=false
skia_enable_pdf=true")

set(SKIA_PRESET_FEATURES_FOR_WIN
"skia_use_egl=false
skia_use_freetype=true
skia_use_system_freetype2=false
skia_use_jpeg_gainmaps=true
skia_use_libjpeg_turbo_decode=true
skia_use_libjpeg_turbo_encode=true
skia_use_zlib=true
skia_use_system_zlib=false
skia_canvaskit_enable_paragraph=true
skia_enable_skparagraph=true
skia_use_libwebp_encode=true
skia_use_icu=true
skia_use_client_icu=true
skia_enable_skunicode=true
skia_enable_gpu=true
skia_enable_fontmgr_custom_embedded=false
skia_canvaskit_enable_canvas_bindings=false
skia_canvaskit_enable_embedded_font=false
skia_use_vulkan=${VULKAN_AVAILABLE}
skia_use_piex=false
skia_use_lua=false
skia_use_dng_sdk=false
skia_use_fontconfig=false
skia_use_libheif=false
skia_use_expat=false
skia_enable_pdf=true
skia_use_system_libjpeg_turbo=false
skia_use_system_harfbuzz=false
skia_use_system_libpng=false
skia_use_system_libwebp=false
skia_use_system_expat=false
skia_use_system_icu=false")

set(SKIA_PRESET_FEATURES_FOR_MACOS
"skia_use_egl=false
skia_use_freetype=true
skia_use_system_freetype2=false
skia_use_system_libjpeg_turbo=false
skia_use_jpeg_gainmaps=true
skia_use_libjpeg_turbo_decode=true
skia_use_libjpeg_turbo_encode=true
skia_use_system_icu=false
skia_use_system_libwebp=false
skia_use_system_libpng=false
skia_use_system_harfbuzz=false
skia_use_zlib=true
skia_use_system_zlib=false
skia_canvaskit_enable_paragraph=true
skia_enable_skparagraph=true
skia_use_libwebp_encode=true
skia_use_icu=true
skia_use_client_icu=true
skia_enable_skunicode=true
skia_enable_gpu=true
skia_enable_fontmgr_custom_embedded=false
skia_canvaskit_enable_canvas_bindings=false
skia_canvaskit_enable_embedded_font=false
skia_enable_ganesh=true
skia_use_vulkan=${VULKAN_AVAILABLE}
skia_use_piex=false
skia_use_lua=false
skia_use_dng_sdk=false
skia_use_fontconfig=false
skia_use_libheif=false
skia_use_system_expat=false
skia_use_expat=false
skia_enable_pdf=true")

if (EMSCRIPTEN)
message(STATUS "EMSDK=$ENV{EMSDK}")
set(SKIA_PRESET_FEATURES_FOR_WASM
"skia_emsdk_dir=\"$ENV{EMSDK}\"
skia_enable_svg=true
skia_canvaskit_enable_paragraph=true
skia_use_jpeg_gainmaps=false
skia_enable_skunicode=true
skia_enable_gpu=true
skia_enable_fontmgr_custom_directory=true
skia_enable_fontmgr_custom_embedded=false
skia_canvaskit_enable_canvas_bindings=false
skia_canvaskit_enable_embedded_font=false
skia_enable_pdf=true

skia_use_angle=false
skia_use_dng_sdk=false
skia_use_dawn=false
skia_use_webgl=true
skia_use_webgpu=false
skia_use_expat=false
skia_use_fontconfig=false
skia_use_freetype=true
skia_use_libheif=false
skia_use_libjpeg_turbo_decode=true
skia_use_libjpeg_turbo_encode=true
skia_use_no_jpeg_encode=false
skia_use_libpng_decode=true
skia_use_libpng_encode=true
skia_use_no_png_encode=false
skia_use_libwebp_decode=true
skia_use_libwebp_encode=true
skia_use_no_webp_encode=false
skia_use_lua=false
skia_use_piex=false
skia_use_system_freetype2=false
skia_use_system_libjpeg_turbo=false
skia_use_system_libpng=false
skia_use_system_libwebp=false
skia_use_system_zlib=false
skia_use_vulkan=false
skia_use_wuffs=true
skia_use_zlib=true
skia_enable_ganesh=true
skia_build_for_debugger=false
skia_enable_sksl_tracing=true
skia_use_icu=true
skia_use_client_icu=false
skia_use_system_icu=false
skia_use_harfbuzz=true
skia_use_system_harfbuzz=false
skia_use_freetype_woff2=true
skia_enable_skshaper=true
skia_enable_skparagraph=true")
endif()

set(SKIA_PRESET_FEATURES_FOR_IOS
"is_trivial_abi=false
skia_enable_fontmgr_empty=false
skia_enable_ganesh=true
skia_enable_gpu=true
skia_enable_pdf=false
skia_enable_skottie=false
skia_enable_skparagraph=true
skia_enable_skunicode=true
skia_enable_spirv_validation=false
skia_enable_tools=false
skia_use_angle=false
skia_use_client_icu=true
skia_use_dng_sdk=false
skia_use_egl=false
skia_use_expat=false
skia_use_fontconfig=false
skia_use_freetype=true
skia_use_icu=true
skia_use_libheif=false
skia_use_libjpeg_turbo_decode=true
skia_use_libjpeg_turbo_encode=true
skia_use_libpng_decode=true
skia_use_libpng_encode=true
skia_use_libwebp_decode=true
skia_use_libwebp_encode=true
skia_use_lua=false
skia_use_metal=true
skia_use_piex=false
skia_use_system_freetype2=false
skia_use_system_harfbuzz=false
skia_use_system_icu=false
skia_use_system_libjpeg_turbo=false
skia_use_system_libpng=false
skia_use_system_libwebp=false
skia_use_system_zlib=false
skia_use_vulkan=false
skia_use_zlib=true")

set(SKIA_PRESET_FEATURES_FOR_ANDROID
"is_official_build=true
ndk_api=${api_level}
skia_use_system_freetype2=false
skia_use_system_libpng=false
skia_use_system_libjpeg_turbo=false
skia_use_system_harfbuzz=false
skia_use_system_libwebp=false
skia_use_expat=true
skia_use_freetype=true
skia_use_system_expat=false
skia_use_system_icu=false
skia_use_egl=true
skia_use_vulkan=${VULKAN_AVAILABLE}")

cmake_minimum_required(VERSION 3.19) # for string(JSON ...)

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
if(${platform} IN_LIST VGG_LINUX_TARGET_LIST)
  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_LINUX})
    string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
elseif(${platform} IN_LIST VGG_WIN_TARGET_LIST)
  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_WIN})
    string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
elseif(platform STREQUAL "WASM" AND DEFINED EMSCRIPTEN)
  string(APPEND OPTIONS " target_cpu=\"wasm\"")
  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_WASM})
    string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
elseif(platform MATCHES "^macOS")
  if(platform STREQUAL "macos-arm64")
    string(APPEND OPTIONS " target_cpu=\"arm64\"")
  endif()
  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_MACOS})
    string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
elseif(platform MATCHES "^iOS")
  string(APPEND OPTIONS " target_os=\"ios\"")
  string(APPEND OPTIONS " target_cpu=\"arm64\"")

  if(platform STREQUAL "iOS-simulator")
    string(APPEND OPTIONS " ios_use_simulator=true")
  endif()

  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_IOS})
    string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
elseif(platform MATCHES "^Android")
  if(platform STREQUAL "Android-armeabi-v7a")
    string(APPEND OPTIONS " target_cpu=\"arm\"")
  elseif(platform STREQUAL "Android-arm64-v8a")
    string(APPEND OPTIONS " target_cpu=\"arm64\"")
  elseif(platform STREQUAL "Android-x86_64")
    string(APPEND OPTIONS " target_cpu=\"x64\"")
  else()
    message(FATAL_ERROR "target type for skia build is not supported: " ${platform})
  endif()
  string(APPEND OPTIONS " ndk=\"${ANDROID_NDK}\"")
  foreach(OPT ${SKIA_PRESET_FEATURES_FOR_ANDROID})
    string(APPEND OPTIONS " ${OPT}")
  endforeach(OPT)
else()
  message(FATAL_ERROR "target type for skia build is invalid: " ${platform})
endif()

if(platform MATCHES "^iOS") # iOS
  # Begin: extra_cflags=[,
  string(APPEND OPTIONS " extra_cflags=[\"-fvisibility=default\", \"-fembed-bitcode\", \"-mios-version-min=15.0\", \"-flto=full\"")
  if(platform STREQUAL "iOS-simulator")
    string(APPEND OPTIONS ", \"--target=arm64-apple-ios15.0-simulator\"")
  endif()
  string(APPEND OPTIONS "]")
  # End extra_cflags: ]

elseif(NOT ${platform} IN_LIST VGG_WIN_TARGET_LIST) # not in window
# we assume non-window using gcc compatible compiler
if(config STREQUAL "RelWithDebInfo")
  string(APPEND OPTIONS " extra_cflags_cc=[\"-fvisibility=default\", \"-g\", \"-frtti\"]")
else()
  string(APPEND OPTIONS " extra_cflags_cc=[\"-fvisibility=default\", \"-frtti\"]")
endif()

elseif(platform STREQUAL "WASM" AND DEFINED EMSCRIPTEN) # wasm
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
  string(APPEND OPTIONS " is_component_build=false")
endif()

if(MSVC)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    string(APPEND OPTIONS " extra_cflags=[\"/MDd\"]")
  else()
    string(APPEND OPTIONS " extra_cflags=[\"/MD\"]")
  endif()
endif()

# set features for skia
set("${out_options}" ${OPTIONS} PARENT_SCOPE)
endfunction()
