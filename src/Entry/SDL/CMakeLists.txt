add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
if(NOT EMSCRIPTEN)
  find_package(SDL2 REQUIRED)
  message(STATUS "SDL2_DIR:  ${SDL2_INCLUDE_DIRS}   ${SDL2_LIBRARIES}")
  include_directories(${SDL2_INCLUDE_DIRS})
  target_link_libraries(sdl_runtime PRIVATE ${SDL2_LIBRARIES})
endif()
target_link_libraries(sdl_runtime PRIVATE vgg_application vgg_adapter)

find_package(Vulkan)
if(Vulkan_FOUND)
  target_compile_definitions(sdl_runtime PRIVATE VGG_USE_VULKAN)
  target_link_libraries(sdl_runtime PRIVATE Vulkan::Vulkan)
endif()
#target_link_libraries(sdl_runtime PRIVATE SDL2)
target_include_directories(sdl_runtime PRIVATE ${VGG_CONTRIB_ARGPARSE_INCLUDE})

get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
set_target_properties(
  sdl_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                         BUILD_RPATH ${VGG_LAYER_RPATH})
