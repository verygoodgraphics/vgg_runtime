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

add_library(vgg_solution OBJECT
  Model/JsonSchemaValidator.cpp)
target_include_directories(vgg_solution PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/Model
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_VALIJSON_INCLUDE}
)

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
  target_link_libraries(runtime skia miniz quickjs vgg_solution)
elseif (APPLE)
  set_target_properties(runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL -I${SDL2_INCLUDE_DIRS}/.."
    LINK_FLAGS ${SDL2_LIBRARIES}
  )
  target_link_libraries(runtime skia miniz quickjs vgg_solution ${OPENGL_LIBRARIES})
else()
  set_target_properties(runtime PROPERTIES
    COMPILE_FLAGS "-DSK_GL"
  )
  target_link_libraries(runtime skia miniz quickjs vgg_solution SDL2 ${OPENGL_LIBRARIES})
  install(TARGETS runtime skia zlib libpng libwebp libjpeg-turbo freetype)
endif()
