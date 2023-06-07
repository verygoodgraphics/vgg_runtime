add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
if (APPLE)
set_target_properties(sdl_runtime PROPERTIES COMPILE_FLAGS "-DSK_GL"
                                             LINK_FLAGS ${SDL2_LIBRARIES})
target_link_libraries(sdl_runtime layer skia ${OPENGL_LIBRARIES})
target_include_directories(sdl_runtime PRIVATE ${SDL2_INCLUDE_DIRS}/..)
endif()
