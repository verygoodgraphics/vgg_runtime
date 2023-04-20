cmake_minimum_required(VERSION 3.8)
project(vgg_runtime)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# defaults to debug mode unless explicitly set to release
if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# customized platform checking flags
#introduce customized cmake vars, see VggVars.cmake in cmake for details
set(VGG_EGL_BACKEND OFF CACHE BOOL "Build runtime with egl backend for offscreen rendering" FORCE)
if(VGG_EGL_BACKEND)
  set(VGG_VAR_GL_BACKEND "EGL" CACHE STRING "" FORCE)
else()
  set(VGG_VAR_GL_BACKEND "SDL" CACHE STRING "" FORCE)
endif()
include(VGGVars)


# find system libs
if (NOT EMSCRIPTEN)
  # find OpenGL library
  cmake_policy(SET CMP0072 NEW)
  find_package(OpenGL REQUIRED)
  message(STATUS "OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
  message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")

  # find SDL2 library
  set(SDL2_BUILDING_LIBRARY ON)
  find_package(SDL2 REQUIRED)
  message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
  message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
endif()

# add bundled libs
add_subdirectory(lib)
include_directories(${VGG_LIB_INCLUDE_PATH})

# add main targets
add_subdirectory(src)
include_directories(${VGG_SRC_INCLUDE_PATH})


# add test targets
include(CTest)
add_subdirectory(test)

get_directory_property(PreprocessorDefs COMPILE_DEFINITIONS)
message("Preprocessor definitions: ${PreprocessorDefs}")
