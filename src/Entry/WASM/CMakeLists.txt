add_executable(vgg_runtime "main.cpp" "WasmBindings.cpp")
  set_target_properties(vgg_runtime PROPERTIES
    COMPILE_FLAGS "-s USE_SDL=2 -fexceptions"
    LINK_FLAGS "\
      -s MODULARIZE=1 \
      -s EXPORT_NAME=\"_vgg_createWasmInstance\" \
      -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 \
      -s ENVIRONMENT=web \
      -s USE_WEBGL2=1 \
      -s USE_SDL=2 \
      -s NO_DISABLE_EXCEPTION_CATCHING \
      -s ASSERTIONS=1 \
      -s INITIAL_MEMORY=64MB \
      -s ALLOW_MEMORY_GROWTH=1 \
      -s STACK_SIZE=32MB \
      --no-heap-copy \
      -s EXPORTED_FUNCTIONS=\['_load_file_from_mem','_is_latest_version','_emscripten_main'\] \
      -s EXPORTED_RUNTIME_METHODS=\['ccall','cwrap','writeArrayToMemory','stringToUTF8'\] \
      --preload-file ../../../../asset \
    "
  )

target_link_libraries(vgg_runtime PRIVATE
  vgg_flags
  vgg_internal
  SDL2
)
set_target_properties(vgg_runtime PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
