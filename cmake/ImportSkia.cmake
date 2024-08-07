# SKIA_SOURCE_DIR is for saving uncompressed skia source code
set(SKIA_SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/skia")

function(prepare_skia_source_dir)
  file(READ ${CMAKE_SOURCE_DIR}/config/DepsConfig.json DEPS_CONFIG)

  string(JSON SKIA_URL ERROR_VARIABLE SKIA_URL_NOTFOUND GET ${DEPS_CONFIG} skia url)
  string(JSON SKIA_ARCHIVE_MD5_CHECKSUM ERROR_VARIABLE SKIA_ARCHIVE_MD5_CHECKSUM_NOTFOUND GET ${DEPS_CONFIG} skia md5)
  if(SKIA_URL_NOTFOUND OR SKIA_ARCHIVE_MD5_CHECKSUM_NOTFOUND)
    message(FATAL_ERROR "Skia url or md5 not found in config/DepsConfig.json")
  endif()
  message(STATUS "SKIA_URL=${SKIA_URL}")
  message(STATUS "SKIA_ARCHIVE_MD5_CHECKSUM=${SKIA_ARCHIVE_MD5_CHECKSUM}")

  string(JSON SKIA_ARCHIVE_FILENAME ERROR_VARIABLE SKIA_ARCHIVE_FILENAME_NOTFOUND GET ${DEPS_CONFIG} skia filename)
  if(SKIA_ARCHIVE_FILENAME_NOTFOUND)
    get_filename_component(SKIA_ARCHIVE_FILENAME ${SKIA_URL} NAME CACHE STRING "" FORCE)
  endif()
  message(STATUS "SKIA_ARCHIVE_FILENAME=${SKIA_ARCHIVE_FILENAME}")

  set(SKIA_ARCHIVE_LOCATION "${CMAKE_SOURCE_DIR}/downloads/${SKIA_ARCHIVE_FILENAME}")
  message(STATUS "Downloading ${SKIA_URL} to ${SKIA_ARCHIVE_LOCATION}")
  file(DOWNLOAD ${SKIA_URL} ${SKIA_ARCHIVE_LOCATION}
    EXPECTED_HASH MD5=${SKIA_ARCHIVE_MD5_CHECKSUM}
    STATUS DOWNLOAD_STATUS
    SHOW_PROGRESS
  )
  if(NOT DOWNLOAD_STATUS MATCHES "^0;")
    message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
  endif()

  file(REMOVE_RECURSE ${SKIA_SOURCE_DIR})
  file(ARCHIVE_EXTRACT INPUT ${SKIA_ARCHIVE_LOCATION} DESTINATION ${SKIA_SOURCE_DIR})
endfunction()

# SKIA_DIR is for user to provide a custom skia directory
# SKIA_EXTERNAL_PROJECT_DIR if for internal use
#
# case 1: user provides a custom skia directory
#
if(DEFINED SKIA_DIR)
  if(NOT IS_DIRECTORY ${SKIA_DIR})
    message(FATAL_ERROR "SKIA_DIR: ${SKIA_DIR} is not a directory")
  endif()
  set(SKIA_EXTERNAL_PROJECT_DIR ${SKIA_DIR} CACHE STRING "" FORCE)
#
# case 2: no skia source code is prepared, so we download and uncompress it
#
elseif(NOT IS_DIRECTORY ${SKIA_SOURCE_DIR})
  prepare_skia_source_dir()
  set(SKIA_EXTERNAL_PROJECT_DIR ${SKIA_SOURCE_DIR} CACHE STRING "" FORCE)
#
# case 3: skia source is already prepared before
#
else()
  file(READ ${CMAKE_SOURCE_DIR}/config/DepsConfig.json DEPS_CONFIG)

  string(JSON SKIA_ARCHIVE_COMMIT ERROR_VARIABLE SKIA_ARCHIVE_COMMIT_NOTFOUND GET ${DEPS_CONFIG} skia commit)
  if (SKIA_ARCHIVE_COMMIT_NOTFOUND)
    message(WARNING "Skia commit id not found in config/DepsConfig.json. Skipped skia directory checking...")
  else()
    message(STATUS "Expected skia commit: ${SKIA_ARCHIVE_COMMIT}")
    file(READ ${SKIA_SOURCE_DIR}/COMMIT_ID SKIA_DIRECTORY_COMMIT_FILE)
    string(STRIP "${SKIA_DIRECTORY_COMMIT_FILE}" SKIA_DIRECTORY_COMMIT)
    message(STATUS "Actual skia commit:   ${SKIA_DIRECTORY_COMMIT}")
    if(NOT SKIA_DIRECTORY_COMMIT STREQUAL "${SKIA_ARCHIVE_COMMIT}")
      prepare_skia_source_dir()
    else()
      message(STATUS "Skia commit is consistent! Skipping re-preparing skia source dir...")
    endif()
  endif()

  set(SKIA_EXTERNAL_PROJECT_DIR ${SKIA_SOURCE_DIR} CACHE STRING "" FORCE)
endif()

message(STATUS "SKIA_EXTERNAL_PROJECT_DIR=${SKIA_EXTERNAL_PROJECT_DIR}")

if(MSVC)
  set(GN "bin/gn.exe")
else()
  set(GN "bin/gn")
endif()

# test gn if it's for the current platform
message(STATUS "Testing ${SKIA_EXTERNAL_PROJECT_DIR}/${GN}...")
if(NOT EXISTS "${SKIA_EXTERNAL_PROJECT_DIR}/${GN}")
  execute_process(COMMAND "python3" "bin/fetch-gn" WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR} COMMAND_ERROR_IS_FATAL ANY)
else()
  message(STATUS "${SKIA_EXTERNAL_PROJECT_DIR}/${GN} exists. Testing if runnable...")
  execute_process(COMMAND "${GN}" --version WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR} RESULT_VARIABLE GN_RUN_RESULT)
  if(NOT GN_RUN_RESULT EQUAL 0)
     message(STATUS "Try to fetch the correct gn executable...")
     execute_process(COMMAND "python3" "bin/fetch-gn" WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR} COMMAND_ERROR_IS_FATAL ANY)
  endif()
endif()

unset(NINJA_COMMAND CACHE)
find_program(NINJA_COMMAND ninja)
if(NOT NINJA_COMMAND)
    message(FATAL_ERROR "ninja is not found. Check your ninja installation.")
endif()
message(STATUS "${NINJA_COMMAND} is found.")

set(SKIA_LIB_LINK_TYPE "dynamic")
if(VGG_VAR_TARGET STREQUAL "WASM"
  OR VGG_VAR_TARGET MATCHES "^iOS"
  OR VGG_VAR_TARGET MATCHES "^Android")
    set(SKIA_LIB_LINK_TYPE "static")
endif()

# if(MSVC)
#   # if SKIA_LIB_LINK_TYPE is dynamic, msvc will not generate skparagraph.lib, so make it static
#   set(SKIA_LIB_LINK_TYPE "static")
# endif()

if(MSVC)
  # use appropriate length
  set(SKIA_LIB_BUILD_PREFIX "out/${SKIA_LIB_LINK_TYPE}/${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
else()
  set(SKIA_LIB_BUILD_PREFIX "out/${VGG_VAR_TARGET}/${ENV_SHELL_PREFIX}/${CMAKE_CXX_COMPILER_ID}_${CMAKE_CXX_COMPILER_VERSION}/${SKIA_LIB_LINK_TYPE}/${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
endif()

message(STATUS "Skia build to: ${SKIA_EXTERNAL_PROJECT_DIR}/${SKIA_LIB_BUILD_PREFIX}")
set(SKIA_LIB_DIR "${SKIA_EXTERNAL_PROJECT_DIR}/${SKIA_LIB_BUILD_PREFIX}" CACHE STRING "" FORCE)
set(SKIA_INCLUDE_DIRS "${SKIA_EXTERNAL_PROJECT_DIR}" "${SKIA_EXTERNAL_PROJECT_DIR}/include/" CACHE STRING "" FORCE)
set(SKIA_LIBS)

include(SkiaUtils)

# setup features for skia compilation for different platform
get_skia_gn_config(CONFIG_OPTIONS ${CMAKE_BUILD_TYPE} ${VGG_VAR_TARGET} ${SKIA_LIB_LINK_TYPE})
string(REPLACE " " ";" PRINT_CONFIG_OPTIONS ${CONFIG_OPTIONS})
foreach(OPT ${PRINT_CONFIG_OPTIONS})
  message(STATUS ${OPT})
endforeach(OPT)

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
  if(VGG_VAR_TARGET STREQUAL "WASM"
    OR VGG_VAR_TARGET MATCHES "^Android")
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
    message(FATAL_ERROR "${LIB_NAME} not found in " ${SKIA_LIB_DIR})
  endif()
endforeach(LIB_NAME)
