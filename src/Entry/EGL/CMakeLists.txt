add_library(egl_runtime EGLRuntime.cpp)
set_target_properties(egl_runtime PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(egl_runtime OpenGL::EGL vgg_layer vgg_domain)

install(TARGETS egl_runtime DESTINATION lib COMPONENT egl_runtime)
install(FILES EGLRuntime.h DESTINATION include COMPONENT egl_runtime)
