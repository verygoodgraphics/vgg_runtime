include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(VGG_SRC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_library(vgg_internal OBJECT
  Controller/Controller.cpp
  Exec/VggExec.cpp
  Model/Loader/ZipLoader.cpp
  Model/JsonDocument.cpp
  Model/JsonSchemaValidator.cpp
  Model/Saver/DirSaver.cpp
  Model/Saver/ZipSaver.cpp
  Model/SchemaValidJsonDocument.cpp
  Model/SubjectJsonDocument.cpp
  Model/VggWork.cpp
  Sdk/VggSdk.cpp
  PlatformAdapter/Helper/StringHelper.cpp
  View/UIView.cpp
)
target_include_directories(vgg_internal PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${VGG_CONTRIB_BOOST_INCLUDE}
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_RXCPP_INCLUDE}
  ${VGG_CONTRIB_VALIJSON_INCLUDE}
)

add_subdirectory(Layer)
target_link_libraries(vgg_internal
  automerge
  vgg_layer
  zip
)
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
  target_link_libraries(vgg_internal vgg_libnode vgg_sketch_parser)
  target_sources(vgg_internal PRIVATE
    PlatformAdapter/Native/Composer/MainComposer.cpp
    PlatformAdapter/Native/Exec/NativeExec.cpp
    PlatformAdapter/Native/Exec/NativeExecImpl.cpp
    PlatformAdapter/Native/Sdk/VggSdkAddon.cpp
    PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.cpp
  )
  target_compile_definitions(vgg_internal PRIVATE NODE_WANT_INTERNALS=1)
endif()
add_subdirectory(Entry)
