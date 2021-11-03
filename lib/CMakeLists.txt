add_subdirectory(skia)
add_subdirectory(quickjs)
option(ENABLE_MINIZ_CPP_TEST OFF)
option(ENABLE_MINIZ_CPP_EXAMPLE OFF)
add_subdirectory(miniz-cpp)

set(VGG_LIB_INCLUDE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/skia # for skia lib itself
  PARENT_SCOPE
)
