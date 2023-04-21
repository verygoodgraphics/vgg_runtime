include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(VGG_SRC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_executable(runtime
  runtime.cpp
  Entity/EntityManager.cpp
  Systems/GeometrySystem.cpp
  Systems/RenderSystem.cpp
  Utils/EntityContainer.cpp
  Utils/WasmBindings.cpp
)

add_library(vgg_internal OBJECT
  Controller/Controller.cpp
  Exec/VggExec.cpp
  Model/DocumentModel.cpp
  Model/JsonDocument.cpp
  Model/JsonSchemaValidator.cpp
  Model/SchemaValidJsonDocument.cpp
  Model/SubjectJsonDocument.cpp
  Model/VggWork.cpp
  Sdk/VggSdk.cpp
  PlatformAdapter/Helper/StringHelper.cpp
)
target_include_directories(vgg_internal PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/Api
  ${CMAKE_CURRENT_SOURCE_DIR}/Controller
  ${CMAKE_CURRENT_SOURCE_DIR}/Exec
  ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure
  ${CMAKE_CURRENT_SOURCE_DIR}/Model
  ${CMAKE_CURRENT_SOURCE_DIR}/Sdk
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_VALIJSON_INCLUDE}
  PlatformAdapter/Helper
)
target_link_libraries(vgg_internal
  automerge
  miniz
  skia 
)
if (EMSCRIPTEN)
  target_sources(vgg_internal PRIVATE
    PlatformAdapter/Browser/Exec/BrowserJSEngine.cpp
    PlatformAdapter/Browser/Sdk/BrowserSdkAdapter.cpp
  )
  target_include_directories(vgg_internal PUBLIC
    PlatformAdapter/Browser/Exec
  )
  target_link_options(vgg_internal PUBLIC -lembind)
else()
  target_link_libraries(vgg_internal vgg_libnode)
  target_sources(vgg_internal PRIVATE
    PlatformAdapter/Native/Composer/MainComposer.cpp
    PlatformAdapter/Native/Exec/NativeExec.cpp
    PlatformAdapter/Native/Exec/NativeExecImpl.cpp
    PlatformAdapter/Native/Sdk/VggSdkAddon.cpp
    PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.cpp
  )
  target_include_directories(vgg_internal PUBLIC
    PlatformAdapter/Native/Composer
    PlatformAdapter/Native/Exec
    PlatformAdapter/Native/Sdk
  )
  target_compile_definitions(vgg_internal PRIVATE NODE_WANT_INTERNALS=1)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set_source_files_properties(
    runtime.cpp
    Utils/WasmBindings.cpp
    PROPERTIES
    COMPILE_FLAGS -DGIT_SHA1=${GIT_SHA1}
  )
endif()
set_target_properties(runtime PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
if (EMSCRIPTEN)
  set_target_properties(runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL -s USE_SDL=2 -fexceptions"
    LINK_FLAGS "\
      -s MODULARIZE=1 \
      -s EXPORT_NAME=\"createModule\" \
      -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 \
      -s ENVIRONMENT=web \
      -s USE_WEBGL2=1 \
      -s USE_SDL=2 \
      -s NO_DISABLE_EXCEPTION_CATCHING \
      -s ASSERTIONS=1 \
      -s ALLOW_MEMORY_GROWTH=1 \
      --no-heap-copy \
      -s EXPORTED_FUNCTIONS=\['_load_file_from_mem','_is_latest_version','_emscripten_main'] \
      -s EXPORTED_RUNTIME_METHODS=\['ccall','cwrap','writeArrayToMemory','stringToUTF8'\] \
    "
    SUFFIX ".js"
  )
elseif (APPLE)
  set_target_properties(runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL -I${SDL2_INCLUDE_DIRS}/.."
    LINK_FLAGS ${SDL2_LIBRARIES}
  )
  target_link_libraries(vgg_internal ${OPENGL_LIBRARIES})
else()
  set_target_properties(runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL"
  )
  target_link_libraries(vgg_internal SDL2 ${OPENGL_LIBRARIES})
  install(TARGETS runtime skia zlib libpng libwebp libjpeg-turbo freetype)
endif()

target_link_libraries(runtime quickjs vgg_internal)