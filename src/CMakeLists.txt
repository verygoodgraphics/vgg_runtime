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
target_include_directories(vgg_internal PRIVATE ${SDL2_INCLUDE_DIRS}/..)
set_target_properties(vgg_internal PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (EMSCRIPTEN)
  target_sources(vgg_internal PRIVATE
    PlatformAdapter/Browser/Exec/BrowserJSEngine.cpp
    PlatformAdapter/Browser/Sdk/BrowserSdkAdapter.cpp
  )
  target_compile_options(vgg_internal PUBLIC 
    -fexceptions
  )
  target_link_options(vgg_internal PUBLIC
    -fexceptions
    -lembind)
else()
  target_link_libraries(vgg_internal vgg_libnode)
  target_sources(vgg_internal PRIVATE
    PlatformAdapter/Native/Exec/NativeExec.cpp
    PlatformAdapter/Native/Exec/NativeExecImpl.cpp
    PlatformAdapter/Native/Sdk/VggSdkAddon.cpp
    PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.cpp
  )
  target_compile_definitions(vgg_internal PRIVATE NODE_WANT_INTERNALS=1)
endif()
add_subdirectory(Entry)
