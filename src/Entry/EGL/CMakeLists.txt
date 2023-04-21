add_library(egl_runtime
  EGLRuntime.cpp
  ${CMAKE_SOURCE_DIR}/src/Entity/EntityManager.cpp
  ${CMAKE_SOURCE_DIR}/src/Systems/GeometrySystem.cpp
  ${CMAKE_SOURCE_DIR}/src/Systems/RenderSystem.cpp
  ${CMAKE_SOURCE_DIR}/src/Utils/EntityContainer.cpp
  ${CMAKE_SOURCE_DIR}/src/Utils/WasmBindings.cpp
)

set_target_properties(egl_runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL"
    # POSITION_INDEPENDENT_CODE ON
)

add_executable(egl_entry
EGLEntry.cpp)
target_link_libraries(egl_entry egl_runtime)

target_link_libraries(egl_runtime skia_egl quickjs vgg_internal SDL2 OpenGL::EGL miniz)
install(TARGETS egl_runtime skia_egl zlib libpng libwebp libjpeg-turbo freetype egl_entry)
set_target_properties(egl_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(egl_entry PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})