add_executable(viewer)
target_sources(viewer PRIVATE render.cpp)
target_link_libraries(viewer PRIVATE vgg_utility vgg_layer vgg_application)
if(NOT EMSCRIPTEN)
  find_package(SDL2 REQUIRED)
endif()
target_link_libraries(viewer PRIVATE SDL2::SDL2)
target_include_directories(viewer PRIVATE ${VGG_CONTRIB_ARGPARSE_INCLUDE})

get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
set_target_properties(
  viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                    BUILD_RPATH ${VGG_LAYER_RPATH})

find_package(Vulkan)
if(Vulkan_FOUND)
  add_executable(exporter exporter.cpp)
  target_compile_definitions(exporter PRIVATE VGG_USE_VULKAN)
  target_link_libraries(exporter vgg_exporter vgg_layer)
  target_include_directories(exporter PRIVATE ${VGG_CONTRIB_ARGPARSE_INCLUDE}
                                              ${VGG_CONTRIB_JSON_INCLUDE})
  get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
  set_target_properties(
    exporter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                        BUILD_RPATH ${VGG_LAYER_RPATH})
endif()
