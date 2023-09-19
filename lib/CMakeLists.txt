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
option(open_vgg_layout_test "Disable vgg_layout test" OFF)
add_subdirectory(vgg_contrib/yoga EXCLUDE_FROM_ALL)
add_subdirectory(vgg_layout EXCLUDE_FROM_ALL)

if (NOT EMSCRIPTEN)
  set(NODE_FOLDER "node$ENV{name}${CMAKE_CXX_COMPILER_ID}${CMAKE_CXX_COMPILER_VERSION}" CACHE STRING "node folder name" FORCE)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
    set(NODE_VERSION "v18.15.0")
    set(NODE_MD5_CHECKSUM "a517c1c055ba65757ccb2d446d165335")
    set(DOWNLOAD_URL "https://nodejs.org/dist/${NODE_VERSION}/node-${NODE_VERSION}.tar.gz")
    set(DOWNLOAD_AT_PATH "${CMAKE_SOURCE_DIR}/downloads/node-${NODE_VERSION}.tar.gz")

    message(STATUS "Downloading ${DOWNLOAD_URL} to ${DOWNLOAD_AT_PATH}")
    file(DOWNLOAD
      ${DOWNLOAD_URL}
      ${DOWNLOAD_AT_PATH}
      EXPECTED_MD5 ${NODE_MD5_CHECKSUM}
      STATUS DOWNLOAD_STATUS
      SHOW_PROGRESS)
    if(NOT DOWNLOAD_STATUS MATCHES "^0;")
      message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
    endif()

    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_AT_PATH}
      DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/node-${NODE_VERSION}" "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
    #file(REMOVE ${DOWNLOAD_AT_PATH})
  endif()

  # this is a custom target to build node with its own building system
  # NOTE this target will always be invoked
  cmake_minimum_required(VERSION 3.22) # this is requied for the following invoking
  add_custom_target(vgg_libnode_building
    COMMENT "Build node if neccessary..."
    COMMAND cmake -DNODE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/BuildLibnode.cmake
  )

  # this is an empty library for linking purpose only
  add_library(vgg_libnode STATIC empty.cpp)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "$ENV{name}" STREQUAL "nix-shell")
    target_link_libraries(vgg_libnode INTERFACE atomic)
  endif()
  target_include_directories(vgg_libnode PUBLIC
    ${NODE_FOLDER}/src
    ${NODE_FOLDER}/deps/uv/include
    ${NODE_FOLDER}/deps/v8/include)
  add_dependencies(vgg_libnode vgg_libnode_building)

  # this is a stub library for providing missing interface that must be linked to libnode
  add_library(vgg_libnode_stub STATIC
    ${NODE_FOLDER}/src/node_snapshot_stub.cc
  )
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
      ${NODE_FOLDER}/out/Release/obj
      ${NODE_FOLDER}/out/Release/obj/deps/base64
      ${NODE_FOLDER}/out/Release/obj/deps/brotli
      ${NODE_FOLDER}/out/Release/obj/deps/cares
      ${NODE_FOLDER}/out/Release/obj/deps/histogram
      ${NODE_FOLDER}/out/Release/obj/deps/llhttp
      ${NODE_FOLDER}/out/Release/obj/deps/nghttp2
      ${NODE_FOLDER}/out/Release/obj/deps/ngtcp2
      ${NODE_FOLDER}/out/Release/obj/deps/openssl
      ${NODE_FOLDER}/out/Release/obj/deps/simdutf
      ${NODE_FOLDER}/out/Release/obj/deps/uv
      ${NODE_FOLDER}/out/Release/obj/deps/uvwasi
      ${NODE_FOLDER}/out/Release/obj/deps/zlib
      ${NODE_FOLDER}/out/Release/obj/tools/icu
      ${NODE_FOLDER}/out/Release/obj/tools/v8_gypfiles
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
