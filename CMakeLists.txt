cmake_minimum_required(VERSION 3.8)
project(vgg_runtime)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# defaults to debug mode unless explicitly set to release
if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

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
