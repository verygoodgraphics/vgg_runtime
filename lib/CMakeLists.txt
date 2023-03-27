add_subdirectory(skia)
add_subdirectory(quickjs)
option(ENABLE_MINIZ_CPP_TEST OFF)
option(ENABLE_MINIZ_CPP_EXAMPLE OFF)
add_subdirectory(miniz-cpp)

add_subdirectory(vgg_contrib)
add_subdirectory(vgg_automerge/automerge)

set(VGG_LIB_INCLUDE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/skia # for skia lib itself
  PARENT_SCOPE
)


if (NOT EMSCRIPTEN)
  find_library(NODE_LIBRARY_PATH node
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/node/out/Release"
    NO_DEFAULT_PATH)

  if(NOT NODE_LIBRARY_PATH)
    set(NODE_VERSION "v18.15.0")
    set(DOWNLOAD_URL "https://nodejs.org/dist/${NODE_VERSION}/node-${NODE_VERSION}.tar.gz")
    set(DOWNLOAD_AT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/node.tar.gz")

    message(STATUS "Downloading ${DOWNLOAD_URL} to ${DOWNLOAD_AT_PATH}")
    file(DOWNLOAD
      ${DOWNLOAD_URL}
      ${DOWNLOAD_AT_PATH}
      STATUS DOWNLOAD_STATUS
      SHOW_PROGRESS)
    if(NOT DOWNLOAD_STATUS MATCHES "^0;")
      message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
    endif()
  
    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_AT_PATH}
      DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/node-${NODE_VERSION}" "${CMAKE_CURRENT_SOURCE_DIR}/node")
    file(REMOVE ${DOWNLOAD_AT_PATH})

    execute_process(
      COMMAND ./configure --enable-static
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/node"
      COMMAND_ERROR_IS_FATAL ANY)
    execute_process(
      COMMAND make -j8
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/node"
      COMMAND_ERROR_IS_FATAL ANY)
  endif()

  add_library(vgg_libnode STATIC
    node/src/node_snapshot_stub.cc)
  target_include_directories(vgg_libnode PUBLIC
    node/src
    node/deps/uv/include
    node/deps/v8/include)
  target_link_directories(vgg_libnode PUBLIC
    node/out/Release)
  target_link_libraries(vgg_libnode PRIVATE 
    base64
    base64_neon64
    brotli
    cares
    histogram
    icudata
    icui18n
    icutools
    icuucx
    llhttp
    nghttp2
    nghttp3
    ngtcp2
    node
    openssl
    simdutf
    torque_base
    uv
    uvwasi
    v8_base_without_compiler
    v8_compiler
    v8_init
    v8_initializers
    v8_libbase
    v8_libplatform
    v8_snapshot
    v8_zlib
    zlib
    zlib_adler32_simd
    zlib_arm_crc32
    zlib_inflate_chunk_simd
  )
endif()