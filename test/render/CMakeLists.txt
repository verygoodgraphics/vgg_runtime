add_executable(viewer)
target_sources(viewer PRIVATE render.cpp)
target_link_libraries(viewer PRIVATE vgg_application vgg_adapter vgg_utility)

set_target_properties(viewer PROPERTIES LINK_FLAGS ${SDL2_LIBRARIES})
target_include_directories(viewer PRIVATE
  ${SDL2_INCLUDE_DIRS}/..
  ${VGG_CONTRIB_ARGPARSE_INCLUDE}
)

get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
set_target_properties(
  viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                         BUILD_RPATH ${VGG_LAYER_RPATH})


if(${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_LINUX_TARGET_LIST)
  add_executable(exporter exporter.cpp)
  target_link_libraries(exporter vgg_exporter)
  target_include_directories(exporter PRIVATE ${VGG_CONTRIB_ARGPARSE_INCLUDE})
  get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
  set_target_properties(exporter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} BUILD_RPATH ${VGG_LAYER_RPATH})
endif()
