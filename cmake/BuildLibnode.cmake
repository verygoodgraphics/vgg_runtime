function(build_libnode WORK_DIR)
  if(VGG_VAR_TARGET IN_LIST VGG_WIN_TARGET_LIST)
    file(EXISTS "${WORK_DIR}/out/Release/node.lib" NODE_BUILT)
    if(NODE_BUILT)
      message(STATUS "Node already build.")
    else()
      # Build node in Windows
      execute_process(
        COMMAND ./vcbuild.bat static
        WORKING_DIRECTORY ${WORK_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    endif()
  else()
    execute_process(
      COMMAND ninja -C "${WORK_DIR}/out/Release"
      ERROR_VARIABLE IS_NOT_BUILDABLE
    )

    if (IS_NOT_BUILDABLE)
      # Configure
      execute_process(
        COMMAND ./configure --enable-static --ninja
        WORKING_DIRECTORY ${WORK_DIR}
        COMMAND_ERROR_IS_FATAL ANY)

      # This is a patch if there is only python3 in search path, which is neccessary for building node with ninja
      execute_process(
        COMMAND sed -i".bak" "s/env python$/env python3/" tools/specialize_node_d.py
        WORKING_DIRECTORY "${WORK_DIR}"
        COMMAND_ERROR_IS_FATAL ANY)

      # Build node with ninja
      execute_process(
        COMMAND ninja -C "${WORK_DIR}/out/Release"
        COMMAND_ERROR_IS_FATAL ANY)
    else()
      message(STATUS "Node already build.")
    endif()
  endif()
endfunction()

#build_libnode(${NODE_DIR})
message(STATUS "build node: ${VGG_VAR_TARGET} | ${VGG_WIN_TARGET_LIST}")
