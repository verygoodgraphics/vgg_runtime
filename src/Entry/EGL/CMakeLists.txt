add_library(egl_runtime EGLRuntime.cpp)
set_target_properties(egl_runtime PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(egl_runtime vgg_application vgg_adapter SDL2)
set_target_properties(egl_runtime PROPERTIES LINK_FLAGS ${SDL2_LIBRARIES})
#FIXME vgg_adapter SDL2 dependency is not necessary after refactoring

target_link_libraries(egl_runtime OpenGL::EGL)

install(TARGETS egl_runtime DESTINATION lib COMPONENT egl_runtime)
install(FILES EGLRuntime.h DESTINATION include COMPONENT egl_runtime)
