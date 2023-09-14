add_subdirectory(vgg_contrib/argparse EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/boost EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/glm EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/json EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/rxcpp EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/valijson EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/zip EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/zlib EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/rapidfuzz-cpp EXCLUDE_FROM_ALL)

# add automerge
add_subdirectory(vgg_contrib/sdefl EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/picosha2 EXCLUDE_FROM_ALL)
add_subdirectory(vgg_automerge/automerge EXCLUDE_FROM_ALL)
set(VGG_AUTOMERGE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/vgg_automerge/automerge CACHE PATH "" FORCE)
mark_as_advanced(VGG_AUTOMERGE_INCLUDE)

# layout
add_subdirectory(vgg_contrib/yoga EXCLUDE_FROM_ALL)
add_subdirectory(vgg_layout EXCLUDE_FROM_ALL)

if (NOT EMSCRIPTEN)
  set(NODE_FOLDER "node$ENV{name}" CACHE STRING "node folder name" FORCE)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
    set(NODE_VERSION "v18.15.0")
    set(DOWNLOAD_URL "https://nodejs.org/dist/${NODE_VERSION}/node-${NODE_VERSION}.tar.gz")
    set(DOWNLOAD_AT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/node.tar.gz")

    message(STATUS "Downloading ${DOWNLOAD_URL} to ${DOWNLOAD_AT_PATH}")
    file(DOWNLOAD
      ${DOWNLOAD_URL}
      ${DOWNLOAD_AT_PATH}
      EXPECTED_MD5 a517c1c055ba65757ccb2d446d165335
      STATUS DOWNLOAD_STATUS
      SHOW_PROGRESS)
    if(NOT DOWNLOAD_STATUS MATCHES "^0;")
      message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
    endif()

    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_AT_PATH}
      DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/node-${NODE_VERSION}" "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
    file(REMOVE ${DOWNLOAD_AT_PATH})
  endif()

  add_library(vgg_libnode STATIC empty.cpp)
  add_library(vgg_libnode_stub STATIC node/src/node_snapshot_stub.cc)

  cmake_minimum_required(VERSION 3.22) # this is requied for the following invoking
  add_custom_command(TARGET vgg_libnode
    PRE_BUILD
    COMMAND cmake -DNODE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/BuildLibnode.cmake
    COMMENT "Build libnode if needed"
  )

  target_include_directories(vgg_libnode PUBLIC
    ${NODE_FOLDER}/src
    ${NODE_FOLDER}/deps/uv/include
    ${NODE_FOLDER}/deps/v8/include)
  target_include_directories(vgg_libnode_stub PUBLIC
    ${NODE_FOLDER}/src
    ${NODE_FOLDER}/deps/uv/include
    ${NODE_FOLDER}/deps/v8/include)
  if(APPLE)
    target_link_directories(vgg_libnode PUBLIC ${NODE_FOLDER}/out/Release)
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
      vgg_libnode_stub
    )
  else()
    target_link_directories(vgg_libnode PUBLIC
      ${NODE_FOLDER}/out/Release/obj.target
      ${NODE_FOLDER}/out/Release/obj.target/deps/base64
      ${NODE_FOLDER}/out/Release/obj.target/deps/brotli
      ${NODE_FOLDER}/out/Release/obj.target/deps/cares
      ${NODE_FOLDER}/out/Release/obj.target/deps/histogram
      ${NODE_FOLDER}/out/Release/obj.target/deps/llhttp
      ${NODE_FOLDER}/out/Release/obj.target/deps/nghttp2
      ${NODE_FOLDER}/out/Release/obj.target/deps/ngtcp2
      ${NODE_FOLDER}/out/Release/obj.target/deps/openssl
      ${NODE_FOLDER}/out/Release/obj.target/deps/simdutf
      ${NODE_FOLDER}/out/Release/obj.target/deps/uv
      ${NODE_FOLDER}/out/Release/obj.target/deps/uvwasi
      ${NODE_FOLDER}/out/Release/obj.target/deps/zlib
      ${NODE_FOLDER}/out/Release/obj.target/tools/icu
      ${NODE_FOLDER}/out/Release/obj.target/tools/v8_gypfiles
    )
    target_link_libraries(vgg_libnode PRIVATE
      "-Wl,--whole-archive"
      base64
      base64_avx
      base64_avx2
      base64_sse41
      base64_sse42
      base64_ssse3
      brotli
      cares
      histogram
      icudata
      icui18n
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
      v8_initializers
      v8_libbase
      v8_libplatform
      v8_snapshot
      v8_zlib
      zlib
      vgg_libnode_stub
      "-Wl,--no-whole-archive"
    )
  endif()
endif()
