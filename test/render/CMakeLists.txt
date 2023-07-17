add_executable(sdl_renderer)
target_sources(sdl_renderer PRIVATE render.cpp)
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


if(NOT VGG_VAR_PLATFORM_TARGET STREQUAL "macOS-apple_silicon")
  if(VGG_ENABLE_EGL)
    add_executable(egl_entry offline_render.cpp)
    target_link_libraries(egl_entry egl_runtime)
    target_include_directories(egl_entry PRIVATE ${VGG_CONTRIB_ARGPARSE_INCLUDE})
    get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
    set_target_properties(egl_entry PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} BUILD_RPATH ${VGG_LAYER_RPATH})
  endif()
endif()
