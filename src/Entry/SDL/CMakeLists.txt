add_executable(sdl_runtime)
target_sources(sdl_runtime PRIVATE "main.cpp")
target_link_libraries(sdl_runtime layer ${SK_LIBS} ${OPENGL_LIBRARIES})
target_include_directories(sdl_runtime PRIVATE ${SDL2_INCLUDE_DIRS}/.. ${SKIA_INCLUDE_DIRS})
