# generate header for VGGVersion.h.in
file(READ ${CMAKE_SOURCE_DIR}/config/globalConstant.json GLOBAL_CONSTANT)
string(JSON version GET ${GLOBAL_CONSTANT} formatVersion)
message(STATUS "Parse for version: " ${version} ${GLOBAL_CONSTANT})
set(VGG_PARSE_FORMAT_VER_STR ${version})
configure_file(${CMAKE_SOURCE_DIR}/cmake/configure.in/VGGVersion.h.in VGGVersion_generated.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})
