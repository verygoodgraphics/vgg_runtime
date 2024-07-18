add_executable(viewer)
target_sources(viewer PRIVATE viewer.cpp)
target_link_libraries(viewer PRIVATE vgg_utility vgg_layer vgg_application)
if(NOT EMSCRIPTEN)
  find_package(SDL2 REQUIRED)
endif()

if(NOT MSVC)
  target_link_libraries(viewer PRIVATE SDL2::SDL2)
else()
  target_include_directories(viewer PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(viewer PRIVATE ${SDL2_LIBRARIES})
  target_compile_definitions(viewer PRIVATE VGG_RUNTIME_DLL_EXPORTING)
endif()

set(ENABLE_IMGUI OFF)
if(ENABLE_IMGUI)
  # For simplicity, we don't provide imgui source code. If you need ImGUI,
  # download and put them in the imgui directory, like the how the cmake use it
  # and set the ENABLE_IMGUI as ON
  target_include_directories(viewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
  file(GLOB IMGUI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui*.cpp)
  set(IMGUI_IMPL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp)
  target_sources(viewer PRIVATE ${IMGUI_SRC} ${IMGUI_IMPL_SRC})
  target_compile_definitions(viewer PRIVATE -DIMGUI_ENABLED)
endif()

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

if(MSVC)
  add_custom_command(TARGET viewer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SKIA_LIB_DIR}/skia.dll"
    "${SKIA_LIB_DIR}/skparagraph.dll"
    "${SKIA_LIB_DIR}/skshaper.dll"
    "${SKIA_LIB_DIR}/sktext.dll"
    "${SKIA_LIB_DIR}/skunicode.dll"
    "${SKIA_EXTERNAL_PROJECT_DIR}/${SKIA_LIB_BUILD_PREFIX}/icudtl.dat"
    "${SDL2_DLL_PATH}"
    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
  )
endif()

if (VGG_VAR_TARGET MATCHES "^Linux")
add_executable(fb_viewer fb_viewer.cpp)
set_target_properties(fb_viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
