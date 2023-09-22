if (NOT EMSCRIPTEN)
  # REFACTOR:: SDL2 dependency is only need by SDL module.
  set(SDL2_BUILDING_LIBRARY ON)
  find_package(SDL2 REQUIRED)
  message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
  message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
endif()
add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
target_link_libraries(sdl_runtime PRIVATE vgg_application vgg_adapter)

find_package(Vulkan)
if(Vulkan_FOUND)
  target_compile_definitions(sdl_runtime PRIVATE VGG_USE_VULKAN)
  target_link_libraries(sdl_runtime PRIVATE Vulkan::Vulkan)
endif()
set_target_properties(sdl_runtime PROPERTIES LINK_FLAGS ${SDL2_LIBRARIES})
target_include_directories(sdl_runtime PRIVATE
  ${SDL2_INCLUDE_DIRS}/..
  ${VGG_CONTRIB_ARGPARSE_INCLUDE}
)

get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
set_target_properties(
  sdl_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                         BUILD_RPATH ${VGG_LAYER_RPATH})
