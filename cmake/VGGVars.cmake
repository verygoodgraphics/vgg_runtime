# This file will output some cmake varibles for configure control and global macros for in C/C++ detection.
#
# --------- INPUT CMake Variables ---------
#
# - VGG_VAR_TARGET: Possible values defined in ${VGG_ALL_TARGETS}. If not provided, it is set automatically depending on the building host.
#
#
# ----------- OUTPUT CMake Variables ---------
#
# The following CMake variable will be defined:
#
# - VGG_VAR_TARGET_PLATFORM will be the prefix of ${VGG_VAR_TARGET}, namely one of the following values:
#   - Linux
#   - Android
#   - Harmony
#   - iOS
#   - macOS
#   - Windows
#   - WASM
#
# - VGG_VAR_TARGET_ARCH will be categorized into one of the following values, depending on the suffix of ${VGG_VAR_TARGET}
#   - X86
#   - ARM
#   - RISCV
#   - WASM
#
# - VGG_VAR_HOST_PLATFORM refers to the host platform, and will be one of the following value:
#   - macOS
#   - Linux
#   - Windows
#
# - VGG_VAR_HOST_ARCH refers to the host cpu architecture, and will be one of the following values:
#   - ARM
#   - X86
#   - RISCV
#
# The VGG_VAR_HOST_PLATFORM and VGG_VAR_HOST_ARCH will be consistent with target variables without cross-compiling.
#
#
# ----------- OUTPUT Compiler Macros ---------
#
# The following compiler macros will be defined:
#
# - VGG_TARGET_${VGG_VAR_TARGET}, with '-' replaced with '_'
# - VGG_TARGET_PLATFORM_${VGG_VAR_TARGET_PLATFORM}
# - VGG_TARGET_ARCH_${VGG_VAR_TARGET_ARCH}
# - VGG_HOST_PLATFORM_${VGG_VAR_HOST_PLATFORM}
# - VGG_HOST_ARCH_${VGG_VAR_HOST_ARCH}

#
# ----------- Example ---------
#
# For example, if VGG_VAR_TARGET is specified as "Android-arm64_v8a", the following variables and macros are defined:
#
# - CMake Variable: VGG_VAR_TARGET_PLATFORM == "Android"
# - CMake Variable: VGG_VAR_TARGET_ARCH == "ARM"
# - CMake Variable: VGG_VAR_HOST_PLATFORM == "macOS" or "Linux" or "Windows", depending on your building OS
# - CMake Variable: VGG_VAR_HOST_ARCH == "ARM" or "X86", depending on your building OS
#
# - Macro: VGG_TARGET_Android_arm64_v8a
# - Macro: VGG_TARGET_PLATFORM_Android
# - Macro: VGG_TARGET_ARCH_ARM
# - Macro: VGG_HOST_PLATFORM_macOS or VGG_HOST_Linux or VGG_Windows, depending on your building OS
# - Macro: VGG_HOST_ARCH_ARM or VGG_HOST_ARCH_X86, depending on your building OS
#
#


###############################################################################

# Apple targets:
list(APPEND VGG_APPLE_TARGET_LIST "iOS" "iOS-simulator" "macOS-arm64" "macOS-x86_64") # iOS defaults to arm64

# Android targets:
# Keep same with Andorid SDK https://developer.android.com/ndk/guides/abis
list(APPEND VGG_ANDROID_TARGET_LIST "Android-armeabi-v7a" "Android-arm64-v8a" "Android-riscv64" "Android-x86_64")

# Harmony targets:
list(APPEND VGG_HARMONY_TARGET_LIST "Harmony-arm64" "Harmony-riscv64" "Harmony-x86_64")

# Linux targets
list(APPEND VGG_LINUX_TARGET_LIST "Linux-x86" "Linux-x86_64" "Linux-arm64" "Linux-riscv64")

# Windows targets
list(APPEND VGG_WIN_TARGET_LIST "Windows-x86" "Windows-x86_64")

# WebAssembly targets
list(APPEND VGG_WASM_TARGET_LIST "WASM") # defaults to WASM32

# All targets
list(APPEND VGG_ALL_TARGETS
  ${VGG_APPLE_TARGET_LIST}
  ${VGG_ANDROID_TARGET_LIST}
  ${VGG_HARMONY_TARGET_LIST}
  ${VGG_LINUX_TARGET_LIST}
  ${VGG_WIN_TARGET_LIST}
  ${VGG_WASM_TARGET_LIST}
)

# Arch Lists
list(APPEND X86_ARCH_LIST "x86" "x86_64" "amd64" "i386" "i486" "i586" "i686" "x86-32" "x86-64")
list(APPEND ARM_ARCH_LIST "arm" "arm64" "armeabi" "armeabi-v7a" "arm64-v8a")
list(APPEND RISCV_ARCH_LIST "riscv64")


###############################################################################

# Detect building host platform
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  set(VGG_VAR_HOST_PLATFORM "macOS" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_HOST_PLATFORM_macOS)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
  # NOTE: When android.toolchain.cmake specified on windows, this branch also reached, A better way for platform detection is necessary.
  set(VGG_VAR_HOST_PLATFORM "Linux" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_HOST_PLATFORM_Linux)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
  set(VGG_VAR_HOST_PLATFORM "Windows" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_HOST_PLATFORM_Windows)
else()
  message(FATAL_ERROR "Unknown host platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# Detect building host arch
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86|x86_64")
  set(VGG_VAR_HOST_ARCH "X86" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_HOST_ARCH_X86)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm|arm64")
  set(VGG_VAR_HOST_ARCH "ARM" CACHE STRING "" FORCE)
  add_compile_definitions(VGG_HOST_ARCH_ARM)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "riscv64")
  set(VGG_VAR_HOST_ARCH "RISCV" CACHE STRING "" FORCE)
else()
  message(FATAL_ERROR "Unknown host arch: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()


###############################################################################

# If emscripten is used, WASM target is forced.
if(EMSCRIPTEN)
  set(VGG_VAR_TARGET "WASM" CACHE STRING "" FORCE)
endif()

# WASM target requires emscripten
if(${VGG_VAR_TARGET} MATCHES "WASM" AND NOT EMSCRIPTEN)
  message(FATAL_ERROR "WASM target requires emscripten SDK. See https://emscripten.org/docs/getting_started/downloads.html")
endif()


# Setup target variables
if(NOT VGG_VAR_TARGET)
  # Give default value depending on current building host
  set(VGG_VAR_TARGET "${VGG_VAR_HOST_PLATFORM}-${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "" FORCE)

  # Check target validity
  if(NOT ${VGG_VAR_TARGET} IN_LIST VGG_ALL_TARGETS)
    message(FATAL_ERROR "Unsupported target: ${VGG_VAR_TARGET}\nPlease specify a target. Candidates: ${VGG_ALL_TARGETS}")
  endif()
  message(STATUS "VGG_VAR_TARGET: ${VGG_VAR_TARGET}")

  # Set target platform and arch by host variables
  set(VGG_VAR_TARGET_PLATFORM "${VGG_VAR_HOST_PLATFORM}" CACHE STRING "" FORCE)
  set(VGG_VAR_TARGET_ARCH "${VGG_VAR_HOST_ARCH}" CACHE STRING "" FORCE)
else()
  # Check target validity
  if(NOT ${VGG_VAR_TARGET} IN_LIST VGG_ALL_TARGETS)
    message(FATAL_ERROR "Unsupported target: ${VGG_VAR_TARGET}\nPlease specify a target. Candidates: ${VGG_ALL_TARGETS}")
  endif()
  message(STATUS "VGG_VAR_TARGET: ${VGG_VAR_TARGET}")

  # Deal with special cases first
  if (VGG_VAR_TARGET MATCHES "^iOS")
    set(VGG_VAR_TARGET_PLATFORM "iOS" CACHE STRING "" FORCE)
    set(VGG_VAR_TARGET_ARCH "ARM" CACHE STRING "" FORCE)
  elseif (VGG_VAR_TARGET STREQUAL "WASM")
    set(VGG_VAR_TARGET_PLATFORM "WASM" CACHE STRING "" FORCE)
    set(VGG_VAR_TARGET_ARCH "WASM" CACHE STRING "" FORCE)
  else()
    # Set target platform and arch by VGG_VAR_TARGET
    string(FIND "${VGG_VAR_TARGET}" "-" POS0)
    if (POS0 EQUAL -1)
      message(FATAL_ERROR "Unexpected target: ${VGG_VAR_TARGET}. Maybe you forget to add a special case for it.")
    endif()
    string(SUBSTRING "${VGG_VAR_TARGET}" 0 ${POS0} _TARGET_PART1)
    math(EXPR POS1 "${POS0} + 1")
    string(SUBSTRING "${VGG_VAR_TARGET}" ${POS1} -1 _TARGET_PART2)

    set(VGG_VAR_TARGET_PLATFORM "${_TARGET_PART1}" CACHE STRING "" FORCE)
    if(_TARGET_PART2 IN_LIST X86_ARCH_LIST)
      set(VGG_VAR_TARGET_ARCH "X86" CACHE STRING "" FORCE)
    elseif(_TARGET_PART2 IN_LIST ARM_ARCH_LIST)
      set(VGG_VAR_TARGET_ARCH "ARM" CACHE STRING "" FORCE)
    elseif(_TARGET_PART2 IN_LIST RISCV_ARCH_LIST)
      set(VGG_VAR_TARGET_ARCH "RISCV" CACHE STRING "" FORCE)
    endif()
  endif()
endif()


# Setup target macros
string(REPLACE "-" "_" _TARGET_STRING "${VGG_VAR_TARGET}")
add_compile_definitions(VGG_TARGET_${_TARGET_STRING})
add_compile_definitions(VGG_TARGET_PLATFORM_${VGG_VAR_TARGET_PLATFORM})
add_compile_definitions(VGG_TARGET_ARCH_${VGG_VAR_TARGET_ARCH})

