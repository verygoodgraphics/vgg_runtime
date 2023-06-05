add_library(egl_runtime
  EGLRuntime.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Entity/EntityManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Systems/GeometrySystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Systems/RenderSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Utils/EntityContainer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Utils/WasmBindings.cpp
)

set_target_properties(egl_runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL"
    # POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(egl_runtime layer skia_egl)

add_executable(egl_entry
EGLEntry.cpp)
target_link_libraries(egl_entry egl_runtime)

target_include_directories(egl_runtime PUBLIC ${VGG_CONTRIB_GLM_INCLUDE})
target_link_libraries(egl_runtime quickjs vgg_internal SDL2 OpenGL::EGL)
set_target_properties(egl_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(egl_entry PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
