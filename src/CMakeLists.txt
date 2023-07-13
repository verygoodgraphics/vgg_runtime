include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(VGG_SRC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_subdirectory(Layer)
add_subdirectory(Domain)
add_subdirectory(Application)
add_subdirectory(Adapter)

add_library(vgg_internal INTERFACE)
target_link_libraries(vgg_internal INTERFACE
  vgg_adapter
)
if(NOT EMSCRIPTEN)
  target_link_libraries(vgg_internal INTERFACE
    vgg_sketch_parser
  )
endif()

set_target_properties(vgg_internal PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(Entry)
