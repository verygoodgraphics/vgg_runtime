  function(build_libnode WORK_DIR)
    find_library(NODE_LIBRARY_PATH node
      PATHS "${WORK_DIR}/out/Release"
      NO_DEFAULT_PATH)

    if(NOT NODE_LIBRARY_PATH)
      message("build libnode...")
      execute_process(
        COMMAND ./configure --enable-static 
        WORKING_DIRECTORY ${WORK_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
      execute_process(
        COMMAND make -j8
        WORKING_DIRECTORY ${WORK_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    endif()

  endfunction()

  build_libnode(${NODE_DIR})