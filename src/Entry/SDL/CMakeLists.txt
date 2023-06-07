add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
set_target_properties(sdl_runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL -I${SDL2_INCLUDE_DIRS}/.."
    LINK_FLAGS ${SDL2_LIBRARIES})
target_link_libraries(sdl_runtime layer skia SDL2 ${OPENGL_LIBRARIES})
