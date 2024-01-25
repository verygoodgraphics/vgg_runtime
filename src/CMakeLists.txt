if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-deprecated-declarations -Wno-nonnull -Wno-error=unused-variable -Wno-error=unused-function")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-deprecated-declarations -Wno-error=unused-private-field -Wno-error=unused-variable -Wno-error=unused-function")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # /WX
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_library(vgg_flags INTERFACE)
if (EMSCRIPTEN)
  target_compile_options(vgg_flags INTERFACE
    -fexceptions
  )
  target_link_options(vgg_flags INTERFACE
    -fexceptions
    -lembind
  )
endif()

add_subdirectory(Utility)
add_subdirectory(Layer)
add_subdirectory(Domain)
add_subdirectory(UseCase)
add_subdirectory(Application)
add_subdirectory(Adapter)

add_library(vgg_internal INTERFACE)
target_link_libraries(vgg_internal INTERFACE
  vgg_adapter
  $<TARGET_OBJECTS:vgg_adapter>
  vgg_utility
)

add_subdirectory(Entry)
