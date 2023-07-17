add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
target_link_libraries(sdl_runtime PRIVATE vgg_application vgg_adapter)

set_target_properties(sdl_runtime PROPERTIES LINK_FLAGS ${SDL2_LIBRARIES})
target_include_directories(sdl_runtime PRIVATE
  ${SDL2_INCLUDE_DIRS}/..
  ${VGG_CONTRIB_ARGPARSE_INCLUDE}
)

get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
set_target_properties(
  sdl_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                         BUILD_RPATH ${VGG_LAYER_RPATH})

add_executable(sdl_renderer)
target_sources(sdl_renderer PRIVATE "renderer_only.cpp")
target_link_libraries(sdl_renderer PRIVATE vgg_application vgg_adapter vgg_utility)

set_target_properties(sdl_renderer PROPERTIES LINK_FLAGS ${SDL2_LIBRARIES})
target_include_directories(sdl_renderer PRIVATE
  ${SDL2_INCLUDE_DIRS}/..
  ${VGG_CONTRIB_ARGPARSE_INCLUDE}
)

get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
set_target_properties(
  sdl_renderer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                         BUILD_RPATH ${VGG_LAYER_RPATH})
