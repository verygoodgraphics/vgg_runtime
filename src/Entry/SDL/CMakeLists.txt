add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
target_link_libraries(sdl_runtime PRIVATE vgg_application vgg_adapter)


# workaroud for App.hpp
# App.hpp should no longer be used, use AppBase.hpp instead
# see the VGG_USE_VULKAN usage in App.hpp 
find_package(Vulkan)
if(Vulkan_FOUND AND ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_LINUX_TARGET_LIST)
  target_compile_definitions(sdl_runtime PRIVATE VGG_USE_VULKAN)
  target_link_libraries(sdl_runtime PRIVATE Vulkan::Vulkan)
  target_include_directories(sdl_runtime PRIVATE
    ${Vulkan_INCLUDE_DIR}
  )
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
