cmake_minimum_required(VERSION 3.19)
project(vgg_runtime)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

# customized platform checking flags
# introduce customized cmake vars, see VggVars.cmake in cmake for details
include(VGGVars)
set(ENV_SHELL_PREFIX $ENV{name} CACHE STRING "" FORCE)
if(NOT ENV_SHELL_PREFIX)
  set(ENV_SHELL_PREFIX "default-shell" CACHE STRING "" FORCE)
endif()
message(STATUS "SHELL: " ${ENV_SHELL_PREFIX})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# must fetch before add subdirectory vgg_contrib
option(ENABLE_UNIT_TEST "Enable unit test" OFF)
if(ENABLE_UNIT_TEST)
  include(FetchContent)
  message(STATUS "Downloading gtest...")
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
else()
  message(STATUS "Unit test is disabled. Enable it with option if needed: cmake -DENABLE_UNIT_TEST=ON")
endif()


# add bundled libs
add_subdirectory(lib)

# add main targets
add_subdirectory(src)

# add test targets
include(CTest)
include_directories(src)
add_subdirectory(test)

get_directory_property(PreprocessorDefs COMPILE_DEFINITIONS)
message("Preprocessor definitions: ${PreprocessorDefs}")
