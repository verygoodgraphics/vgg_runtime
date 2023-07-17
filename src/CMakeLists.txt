include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(VGG_SRC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

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

add_subdirectory(Layer)
add_subdirectory(Domain)
add_subdirectory(Application)
add_subdirectory(Adapter)

add_library(vgg_internal INTERFACE)
target_link_libraries(vgg_internal INTERFACE
  vgg_adapter
  $<TARGET_OBJECTS:vgg_adapter>
)

add_subdirectory(Entry)
