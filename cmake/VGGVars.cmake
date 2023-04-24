# This file will output some cmake varibles for configure control and global macros for in C/C++ detection.
#
# --------- CMake Variables Inputs ---------
# VGG_VAR_PLATFORM_TARGET: possible values defined bellow
#
#
# ----------- OUTPUT Variables and Macros ---------
#
# The following CMake variable will be defined:
#
# - VGG_VAR_HOST will be a string containing the prefix of ${VGG_VAR_PLATFORM_TARGET}
#
# - VGG_VAR_ARCH will be one of the following values:
#   - "ARM" and macro VGG_ARCH_ARM will be defined
#   - "X86" and macro VGG_ARCH_X86 will be defined
#   - "WASM" and macro VGG_ARCH_WASM will be defined
#
# - VGG_VAR_BUILDING_OS_STR will be one of the following value:
#   - "macOS" and macro VGG_BUILDING_OS_MACOS defined
#   - "Linux" and macro VGG_BUILDING_OS_LINUX defined
#   - "Windows" and macro VGG_BUILDING_OS_WIN defined
#
# and more macros will be defined:
#
# - VGG_TARGET_${VGG_VAR_PLATFORM_TARGET}: the '-' in VGG_VAR_PLATFORM_TARGET will be replaced by '_'
#
# - VGG_HOST_${the prefix of ${VGG_VAR_PLATFORM_TARGET}}
#
#
#
# For example, if VGG_VAR_PLATFORM_TARGET is specified as "Android-arm64_v8a", the following macros are defined:
#
# - VGG_TARGET_Android_arm64_v8a
#
# - VGG_HOST_Android
#
# and CMake variable VGG_VAR_HOST equals "Android"



# apple targets:
list(APPEND VGG_APPLE_TARGET_LIST "iOS" "iOS-simulator" "macOS-apple_silicon" "macOS-x86" )

# android targets:
# Keep same with Andorid SDK https://developer.android.com/ndk/guides/abis
list(APPEND VGG_ANDROID_TARGET_LIST "Android-armeabi_v7a" "Android-arm64_v8a" "Android-x86" "Android-x86_64")

# Windows targets
list(APPEND VGG_WIN_TARGET_LIST "Windows-x86_64" "Windows-x86")

# linux targets
list(APPEND VGG_LINUX_TARGET_LIST "Linux-x86" "Linux-x86_64")

# other targets
list(APPEND VGG_OTHER_TARGET_LIST "WASM")

list(APPEND VGG_ALL_TARGETS ${VGG_APPLE_TARGET_LIST} ${VGG_ANDROID_TARGET_LIST} ${VGG_WIN_TARGET_LIST} ${VGG_LINUX_TARGET_LIST} ${VGG_OTHER_TARGET_LIST})
list(APPEND X86_ARCH_LIST "Windows-x86_64" "Windows-x86" "Linux-x86" "Linux-x86_64" "Android-x86" "Android-x86_64" "macOS-x86")

if(APPLE)
  set(VGG_VAR_BUILDING_OS_STR "macOS" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_BUILDING_OS_MACOS)
elseif(WIN32)
  set(VGG_VAR_BUILDING_OS_STR "Windows" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_BUILDING_OS_WIN)
elseif(UNIX)
  # NOTE: When android.toolchain.cmake specified on windows, this branch also reached, A better way for platform detection is necessary.
  set(VGG_VAR_BUILDING_OS_STR "Linux" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_BUILDING_OS_LINUX)
endif()

#if emscripten is provided, wasm target is set forced.
if(EMSCRIPTEN)
  set(VGG_VAR_PLATFORM_TARGET "WASM" CACHE STRING "" FORCE)
endif()


# Give default value dependent on current building platform
if(NOT VGG_VAR_PLATFORM_TARGET)
  if(VGG_VAR_BUILDING_OS_STR STREQUAL "macOS")
    set(VGG_VAR_PLATFORM_TARGET "macOS-apple_silicon")
  elseif(VGG_VAR_BUILDING_OS_STR STREQUAL "Windows")
    set(VGG_VAR_PLATFORM_TARGET "Windows-x86_64")
  elseif(VGG_VAR_BUILDING_OS_STR STREQUAL "Linux")
    set(VGG_VAR_PLATFORM_TARGET "Linux-x86_64")
  endif()
endif()

# Variables checking
if(NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_APPLE_TARGET_STR_LIST AND
   NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_WIN_TARGET_LIST AND
   NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_ANDROID_TARGET_LIST AND
   NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_LINUX_TARGET_LIST AND
   NOT ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_OTHER_TARGET_LIST)
  message(FATAL_ERROR "Please specifies a platform target by VGG_VAR_PLATFORM_TARGET. ${VGG_VAR_PLATFORM_TARGET}\n Candidate platforms: ${VGG_APPLE_TARGET_STR_LIST} ${VGG_WIN_TARGET_LIST} ${VGG_ANDROID_TARGET_LIST} ${VGG_LINUX_TARGET_LIST}")
endif()

if(${VGG_VAR_PLATFORM_TARGET} MATCHES "WASM" AND NOT EMSCRIPTEN)
  message(FATAL_ERROR "emscripten tool chain not set, WASM target is invalid\n")
endif()



# preprocessor definition
string(REPLACE "-" "_" _TARGET_PLATROM_STR ${VGG_VAR_PLATFORM_TARGET})
add_compile_definitions(VGG_TARGET_${_TARGET_PLATROM_STR})


string(FIND ${VGG_VAR_PLATFORM_TARGET} "-" POS)
string(SUBSTRING ${VGG_VAR_PLATFORM_TARGET} 0 ${POS} _HOST_PREFIX)
add_compile_definitions(VGG_HOST_${_HOST_PREFIX})
set(VGG_VAR_HOST ${_HOST_PREFIX})

if(${VGG_VAR_PLATFORM_TARGET} IN_LIST X86_ARCH_LIST)
  add_compile_definitions(VGG_ARCH_X86)
  set(VGG_VAR_ARCH "X86" CACHE STRING "" FORCE)
elseif(${VGG_VAR_PLATFORM_TARGET} STREQUAL "WASM")
  add_compile_definitions(VGG_ARCH_WASM)
  set(VGG_VAR_ARCH "WASM" CACHE STRING "" FORCE)
else()
  add_compile_definitions(VGG_ARCH_ARM)
  set(VGG_VAR_ARCH "ARM" CACHE STRING "" FORCE)
endif()


# Setup android.toolchian.cmake variables
if(${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_ANDROID_TARGET_LIST)
  # See https://developer.android.com/ndk/guides/abis
  if(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android-armeabi_v7a")
    set(ANDROID_ABI "armeabi-v7a with NEON")
  elseif(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android-arm64_v8a")
    set(ANDROID_ABI "arm64-v8a")
  elseif(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android-x86")
    set(ANDROID_ABI "x86")
  elseif(${VGG_VAR_PLATFORM_TARGET} MATCHES "Android-x86_64")
    set(ANDROID_ABI "x86_64")
  endif()
endif()


# Setup ios.toolchian.cmake variables
if(${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_APPLE_TARGET_STR_LIST)
  # PLATFORM 是ios.toolchain.cmake 的变量
  if(VGG_VAR_PLATFORM_TARGET STREQUAL "iOS")
    set(PLATFORM "OS64")
    # TODO:: iphone device need a signature to build
  elseif(VGG_VAR_PLATFORM_TARGET STREQUAL "iOS-simulator")
    set(PLATFORM "SIMULATOR64")
  elseif(VGG_VAR_PLATFORM_TARGET STREQUAL "macOS")
    set(PLATFORM "MACOS")
  endif()
endif()
