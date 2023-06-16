# find_library(skia_DEB NAMES skia PATHS "/home/ysl/Code/skia/out/Static")

set(SKIA_EXTERNAL_PROJECT_DIR "/home/ysl/Code/skia/")
set(SKIA_FOR_LINUX_FEATURES "gpu")
set(SKIA_LIB_LINK_TYPE "dynamic")

set(SKIA_LIB_DIR "/home/ysl/Code/skia/out/Linux/Shared/DebugEGL")
set(SKIA_INCLUDE_DIRS "${SKIA_EXTERNAL_PROJECT_DIR}" "${SKIA_EXTERNAL_PROJECT_DIR}/include/")
set(SKIA_LIBS)

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


set(GN "bin/gn")
# ===============================================
# add skia as custome target
set(SKIA_BUILD_FEATURES)
set(SKIA_BUILD_DIR)
include(SkiaUtils)
# setup features for skia compilation for different platform
get_skia_gn_config(CONFIG_OPTIONS "debug" ${VGG_VAR_PLATFORM_TARGET} ${SKIA_LIB_LINK_TYPE})
string(REPLACE " " ";" PRINT_CONFIG_OPTIONS ${CONFIG_OPTIONS})
foreach(OPT ${PRINT_CONFIG_OPTIONS})
  message(STATUS ${OPT})
endforeach(OPT)


# ===============================================

set(SKIA_LIB_NAMES skia)
execute_process(COMMAND ${GN} desc --format=json --all ${SKIA_LIB_DIR} //:skia
  WORKING_DIRECTORY ${SKIA_EXTERNAL_PROJECT_DIR}
  RESULT_VARIABLE CMD_RET
  OUTPUT_VARIABLE SKIA_DUMP_CONFIG
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
  find_library(FOUND_LIB_${LIB_NAME} NAMES ${LIB_NAME} PATHS ${SKIA_LIB_DIR})
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
