if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
elseif(CMAKE_GENERATOR STREQUAL Xcode)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-nonnull -Wno-unused-private-field -Wno-error=unused-function -Wno-maybe-uninitialized -Wno-error=restrict")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-unknown-warning-option -Wno-deprecated-declarations -Wno-nonnull -Wno-unused-private-field -Wno-error=unused-function -Wno-maybe-uninitialized -Wno-error=restrict")
endif()

if (VGG_VAR_TARGET MATCHES "^Android")
  # -fPIC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=protected")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fvisibility=protected")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384 -Wl,--no-allow-shlib-undefined,--no-undefined -Wl,-z,defs,-z,now,-z,relro")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_library(vgg_flags INTERFACE)
target_compile_definitions(vgg_flags INTERFACE GIT_SHA1="${GIT_SHA1}")
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
