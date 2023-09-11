if(NOT DEFINED SKIA_EXTERNAL_PROJECT_DIR)
    message(FATAL_ERROR "SKIA_EXTERNAL_PROJECT_DIR is not provided")
endif()
if(NOT IS_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR})
    message(FATAL_ERROR "SKIA_EXTERNAL_PROJECT_DIR: ${SKIA_EXTERNAL_PROJECT_DIR} is not a directory")
endif()

unset(NINJA_COMMAND CACHE)
find_program(NINJA_COMMAND ninja)
if(NOT NINJA_COMMAND)
    message(FATAL_ERROR "ninja is not found. Check your ninja installation.")
endif()
message(STATUS "${NINJA_COMMAND} is found.")

set(SKIA_LIB_LINK_TYPE "dynamic")
if(VGG_VAR_PLATFORM_TARGET STREQUAL "WASM")
    set(SKIA_LIB_LINK_TYPE "static")
endif()

set(SKIA_LIB_BUILD_PREFIX "out/${VGG_VAR_PLATFORM_TARGET}/${ENV_SHELL_PREFIX}/${CMAKE_CXX_COMPILER_ID}/${SKIA_LIB_LINK_TYPE}/${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
message(STATUS "Skia build to: ${SKIA_LIB_BUILD_PREFIX}")
set(SKIA_LIB_DIR "${SKIA_EXTERNAL_PROJECT_DIR}/${SKIA_LIB_BUILD_PREFIX}" CACHE STRING "" FORCE)
set(SKIA_INCLUDE_DIRS "${SKIA_EXTERNAL_PROJECT_DIR}" "${SKIA_EXTERNAL_PROJECT_DIR}/include/" CACHE STRING "" FORCE)
set(SKIA_LIBS)

set(GN "bin/gn")
include(SkiaUtils)

# setup features for skia compilation for different platform
get_skia_gn_config(CONFIG_OPTIONS ${CMAKE_BUILD_TYPE} ${VGG_VAR_PLATFORM_TARGET} ${SKIA_LIB_LINK_TYPE})
string(REPLACE " " ";" PRINT_CONFIG_OPTIONS ${CONFIG_OPTIONS})
foreach(OPT ${PRINT_CONFIG_OPTIONS})
  message(STATUS ${OPT})
endforeach(OPT)

if(VGG_VAR_PLATFORM_TARGET STREQUAL "macOS-apple_silicon")
  string(APPEND OPTIONS " target_cpu='arm64'")
elseif(VGG_VAR_PLATFORM_TARGET STREQUAL "WASM")
  string(APPEND OPTIONS " target_cpu='wasm'")
endif()
# config skia
execute_process(COMMAND ${GN} gen ${SKIA_LIB_BUILD_PREFIX} ${CONFIG_OPTIONS}
WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR} COMMAND_ERROR_IS_FATAL ANY)

# build skia
execute_process(COMMAND ${NINJA_COMMAND} -C ${SKIA_LIB_BUILD_PREFIX}
  WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR}
  COMMAND_ERROR_IS_FATAL ANY
)


# ===============================================
set(SKIA_LIB_NAMES skia skparagraph skshaper skunicode)
execute_process(COMMAND ${GN} desc --format=json --all ${SKIA_LIB_DIR} //:skia
  WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR}
  RESULT_VARIABLE CMD_RET
  OUTPUT_VARIABLE SKIA_DUMP_CONFIG
  COMMAND_ERROR_IS_FATAL ANY
)

get_definitions(SKIA_COMPILE_DEFS ${SKIA_DUMP_CONFIG} "//:skia")

message(STATUS "Proprocessor defined by skia: ")
foreach(ITEM ${SKIA_COMPILE_DEFS})
  message(STATUS ${ITEM})
endforeach(ITEM)

message(STATUS "libs linked by skia: ")
foreach(ITEM ${SKIA_LINK_LIBS})
  message(STATUS ${ITEM})
endforeach(ITEM)

foreach(LIB_NAME ${SKIA_LIB_NAMES})
  unset(FOUND_LIB_${LIB_NAME} CACHE)
  if(VGG_VAR_PLATFORM_TARGET STREQUAL "WASM")
    set(FOUND_LIB_${LIB_NAME} "${SKIA_LIB_DIR}/lib${LIB_NAME}.a")
  else()
    find_library(FOUND_LIB_${LIB_NAME} NAMES ${LIB_NAME} ${LIB_NAME}.dll PATHS ${SKIA_LIB_DIR})
  endif()
  if(FOUND_LIB_${LIB_NAME})
    message(STATUS "find skia library: ${FOUND_LIB_${LIB_NAME}} in ${SKIA_LIB_DIR}")
    add_library(skia::${LIB_NAME} STATIC IMPORTED)
    set_target_properties(skia::${LIB_NAME} PROPERTIES
      IMPORTED_LOCATION "${FOUND_LIB_${LIB_NAME}}"
      IMPORTED_LOCATION_DEBUG "${FOUND_LIB_${LIB_NAME}}"
      IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    if(NOT SKIA_COMPILE_DEFS STREQUAL "")
      target_compile_definitions(skia::${LIB_NAME} INTERFACE ${SKIA_COMPILE_DEFS})
    endif()
    target_include_directories(skia::${LIB_NAME} INTERFACE ${SKIA_INCLUDE_DIRS})
    list(APPEND SKIA_LIBS skia::${LIB_NAME})
  else()
    message(STATUS "${LIB_NAME} not found in " ${SKIA_LIB_DIR})
  endif()
endforeach(LIB_NAME)
