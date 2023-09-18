  function(build_libnode WORK_DIR)
    find_library(NODE_LIBRARY_PATH node
      PATHS "${WORK_DIR}/out/Release"
      NO_DEFAULT_PATH)

    if(NOT NODE_LIBRARY_PATH)
      message("build libnode...")

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
        COMMAND ninja
        WORKING_DIRECTORY "${WORK_DIR}/out/Release"
        COMMAND_ERROR_IS_FATAL ANY)
    endif()

  endfunction()

  build_libnode(${NODE_DIR})
