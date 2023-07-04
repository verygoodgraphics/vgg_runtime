add_executable(wasm_main "main.cpp" "${CMAKE_SOURCE_DIR}/src/Utils/WasmBindings.cpp")
  set_target_properties(wasm_main PROPERTIES
    COMPILE_FLAGS "-s USE_SDL=2 -fexceptions"
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
      -s EXPORTED_FUNCTIONS=\['_load_file_from_mem','_is_latest_version','_emscripten_main'\] \
      -s EXPORTED_RUNTIME_METHODS=\['ccall','cwrap','writeArrayToMemory','stringToUTF8'\] \
      --preload-file ../../../../asset \
    "
    SUFFIX ".js"
  )

target_link_libraries(wasm_main PRIVATE vgg_internal SDL2)
set_target_properties(wasm_main PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
