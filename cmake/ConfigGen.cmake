# read format version from config
file(READ ${CMAKE_SOURCE_DIR}/config/GlobalConstant.json GLOBAL_CONSTANT)
string(JSON version ERROR_VARIABLE version_NOTFOUND GET ${GLOBAL_CONSTANT} formatVersion)
if (version_NOTFOUND)
  message(FATAL_ERROR "Format version nout found in config/GlobalConstant.json")
endif()
message(STATUS "Parse for format version: " ${version})

# generate header for VGGVersion.h.in
set(VGG_PARSE_FORMAT_VER_STR ${version})
configure_file(${CMAKE_SOURCE_DIR}/cmake/configure.in/VGGVersion.h.in VGGVersion_generated.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})
