function(build_libnode WORK_DIR)

  execute_process(
    COMMAND ninja -C "${WORK_DIR}/out/Release"
    ERROR_VARIABLE IS_NOT_BUILDABLE
  )

  if (IS_NOT_BUILDABLE)
    # Configure
    if (NOT VGG_VAR_TARGET_ARCH STREQUAL "X86")
      execute_process(
        COMMAND ./configure --enable-static --ninja
        WORKING_DIRECTORY ${WORK_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    else()
      execute_process(
        COMMAND ./configure --enable-static --ninja --openssl-no-asm
        WORKING_DIRECTORY ${WORK_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    endif()

    # This is a patch if there is only python3 in search path, which is neccessary for building node with ninja
    execute_process(
      COMMAND sed -i".bak" "s/env python$/env python3/" tools/specialize_node_d.py
      WORKING_DIRECTORY "${WORK_DIR}"
      COMMAND_ERROR_IS_FATAL ANY)

    # This is another workaround because clang 15 on macOS is not able to compile node v18. Upgrade of node is needed in the future
    execute_process(
      COMMAND sed -i".bak" "1s/^/#pragma clang diagnostic ignored \"-Wenum-constexpr-conversion\"/" deps/v8/src/base/bit-field.h
      WORKING_DIRECTORY "${WORK_DIR}"
      COMMAND_ERROR_IS_FATAL ANY)

    # Build node with ninja
    execute_process(
      COMMAND ninja -C "${WORK_DIR}/out/Release"
      COMMAND_ERROR_IS_FATAL ANY)
  else()
    message(STATUS "Node already build.")
  endif()

endfunction()

build_libnode(${NODE_DIR})
