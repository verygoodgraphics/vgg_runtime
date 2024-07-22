add_subdirectory(vgg_contrib/argparse EXCLUDE_FROM_ALL)
add_subdirectory(vgg_contrib/bezier EXCLUDE_FROM_ALL)
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
add_subdirectory(vgg_layout)

if(EMSCRIPTEN)
  return()  # Return from this file
endif()

file(READ ${CMAKE_SOURCE_DIR}/config/DepsConfig.json DEPS_CONFIG)

if(MSVC)
  set(SDL2_DLL_PATH "${CMAKE_SOURCE_DIR}/lib/SDL2-2.28.5/lib/x64/SDL2.dll" CACHE STRING "sdl2 dll position for msvc x64" FORCE)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.28.5")
    string(JSON SDL2_URL ERROR_VARIABLE SDL2_URL_NOTFOUND GET ${DEPS_CONFIG} sdl2 url)
    string(JSON SDL2_FILENAME ERROR_VARIABLE SDL2_FILENAME_NOTFOUND GET ${DEPS_CONFIG} sdl2 filename)
    string(JSON SDL2_ARCHIVE_MD5_CHECKSUM ERROR_VARIABLE SDL2_ARCHIVE_MD5_CHECKSUM_NOTFOUND GET ${DEPS_CONFIG} sdl2 md5)
    if(SDL2_URL_NOTFOUND OR SDL2_FILENAME_NOTFOUND OR SDL2_ARCHIVE_MD5_CHECKSUM_NOTFOUND)
      message(FATAL_ERROR "sdl2 url or filename or md5 not found in config/DepsConfig.json")
    endif()

    set(DOWNLOAD_AT_PATH "${CMAKE_SOURCE_DIR}/downloads/${SDL2_FILENAME}")

    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/downloads/${SDL2_FILENAME}")
      message(STATUS "Downloading ${SDL2_URL} to ${DOWNLOAD_AT_PATH}")
      file(DOWNLOAD ${SDL2_URL} ${DOWNLOAD_AT_PATH}
        EXPECTED_MD5 ${SDL2_ARCHIVE_MD5_CHECKSUM}
        STATUS DOWNLOAD_STATUS SHOW_PROGRESS)
      if(NOT DOWNLOAD_STATUS MATCHES "^0;")
        message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
      endif()
    endif()

    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_AT_PATH} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.28.5/sdl2-config.cmake"
    "set(SDL2_INCLUDE_DIRS \"\${CMAKE_CURRENT_LIST_DIR}/include\")\n"
    "if (\${CMAKE_SIZEOF_VOID_P} MATCHES 8)\n"
    "  set(SDL2_LIBRARIES \"\${CMAKE_CURRENT_LIST_DIR}/lib/x64/SDL2.lib;\${CMAKE_CURRENT_LIST_DIR}/lib/x64/SDL2main.lib\")\n"
    "else ()\n"
    "  set(SDL2_LIBRARIES \"\${CMAKE_CURRENT_LIST_DIR}/lib/x86/SDL2.lib;\${CMAKE_CURRENT_LIST_DIR}/lib/x86/SDL2main.lib\")\n"
    "endif ()\n"
    "string(STRIP \"\${SDL2_LIBRARIES}\" SDL2_LIBRARIES)")
endif()

if(VGG_VAR_TARGET MATCHES "^iOS")
  # Download src
  set(NODE_FOLDER "nodejs-mobile" CACHE STRING "node folder name" FORCE)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
    string(JSON NODEJS_URL ERROR_VARIABLE NODEJS_URL_NOTFOUND GET ${DEPS_CONFIG} nodejs-mobile url)
    string(JSON NODEJS_FILENAME ERROR_VARIABLE NODEJS_FILENAME_NOTFOUND GET ${DEPS_CONFIG} nodejs-mobile filename)
    if(NODEJS_URL_NOTFOUND OR NODEJS_FILENAME_NOTFOUND)
      message(FATAL_ERROR "Nodejs-mobile url or filename not found in config/DepsConfig.json")
    endif()

    set(DOWNLOAD_AT_PATH "${CMAKE_SOURCE_DIR}/downloads/${NODEJS_FILENAME}")

    message(STATUS "Downloading ${NODEJS_URL} to ${DOWNLOAD_AT_PATH}")
    file(DOWNLOAD ${NODEJS_URL} ${DOWNLOAD_AT_PATH} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
    if(NOT DOWNLOAD_STATUS MATCHES "^0;")
      message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
    endif()

    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_AT_PATH} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
    file(GLOB NODEJS_MOBILE_DIR LIST_DIRECTORIES true nodejs-mobile-*)
    file(RENAME ${NODEJS_MOBILE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
  endif()

  # Download the pre-built binary (NodeMobile.xcframework)
  set(LIB_NODE_FOLDER "nodejs-mobile-ios" CACHE STRING "lib node folder name" FORCE)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NODE_FOLDER}")
    string(JSON NODEJS_URL ERROR_VARIABLE NODEJS_URL_NOTFOUND GET ${DEPS_CONFIG} nodejs-mobile ios-binary-url)
    string(JSON NODEJS_FILENAME ERROR_VARIABLE NODEJS_FILENAME_NOTFOUND GET ${DEPS_CONFIG} nodejs-mobile ios-binary-filename)
    if(NODEJS_URL_NOTFOUND OR NODEJS_FILENAME_NOTFOUND)
      message(FATAL_ERROR "Nodejs-mobile ios binary url or filename not found in config/DepsConfig.json")
    endif()

    set(LIB_DOWNLOAD_AT_PATH "${CMAKE_SOURCE_DIR}/downloads/${NODEJS_FILENAME}")

    message(STATUS "Downloading ${NODEJS_URL} to ${LIB_DOWNLOAD_AT_PATH}")
    file(DOWNLOAD ${NODEJS_URL} ${LIB_DOWNLOAD_AT_PATH} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
    if(NOT DOWNLOAD_STATUS MATCHES "^0;")
      message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
    endif()

    file(ARCHIVE_EXTRACT INPUT ${LIB_DOWNLOAD_AT_PATH} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
    file(GLOB LIB_NODEJS_MOBILE_DIR LIST_DIRECTORIES true nodejs-mobile-*-ios)
    file(RENAME ${LIB_NODEJS_MOBILE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${LIB_NODE_FOLDER}")
  endif()

else()
  set(NODE_FOLDER "node$ENV{name}${CMAKE_CXX_COMPILER_ID}${CMAKE_CXX_COMPILER_VERSION}" CACHE STRING "node folder name" FORCE)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
    string(JSON NODEJS_URL ERROR_VARIABLE NODEJS_URL_NOTFOUND GET ${DEPS_CONFIG} nodejs url)
    string(JSON NODEJS_FILENAME ERROR_VARIABLE NODEJS_FILENAME_NOTFOUND GET ${DEPS_CONFIG} nodejs filename)
    string(JSON NODEJS_ARCHIVE_MD5_CHECKSUM ERROR_VARIABLE NODEJS_ARCHIVE_MD5_CHECKSUM_NOTFOUND GET ${DEPS_CONFIG} nodejs md5)
    if(NODEJS_URL_NOTFOUND OR NODEJS_FILENAME_NOTFOUND OR NODEJS_ARCHIVE_MD5_CHECKSUM_NOTFOUND)
      message(FATAL_ERROR "Nodejs url or filename or md5 not found in config/DepsConfig.json")
    endif()

    set(DOWNLOAD_AT_PATH "${CMAKE_SOURCE_DIR}/downloads/${NODEJS_FILENAME}")

    message(STATUS "Downloading ${NODEJS_URL} to ${DOWNLOAD_AT_PATH}")
    file(DOWNLOAD ${NODEJS_URL} ${DOWNLOAD_AT_PATH}
      EXPECTED_MD5 ${NODEJS_ARCHIVE_MD5_CHECKSUM}
      STATUS DOWNLOAD_STATUS SHOW_PROGRESS)
    if(NOT DOWNLOAD_STATUS MATCHES "^0;")
      message(FATAL_ERROR "Downloading failed with ${DOWNLOAD_STATUS}")
    endif()

    file(ARCHIVE_EXTRACT INPUT ${DOWNLOAD_AT_PATH} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
    file(GLOB NODEJS_DIR LIST_DIRECTORIES true node-*)
    file(RENAME ${NODEJS_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}")
  endif()

  if(NOT MSVC)
    # this is a custom target to build node with its own building system
    cmake_minimum_required(VERSION 3.22) # this is requied for the following invoking
    add_custom_target(vgg_libnode_building
      COMMENT "Build node if neccessary..."
      COMMAND cmake -DNODE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/BuildLibnode.cmake
    )
  elseif(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NODE_FOLDER}/out/${CMAKE_BUILD_TYPE}/node.exe")
    message(STATUS "start build node...")
    # 1.node need vs2022
    # 2.In windows, can not apply release mode DLL to other debug mode project
    execute_process(COMMAND "vcbuild.bat" ${CMAKE_BUILD_TYPE} "vs2022" "dll" "x64" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${NODE_FOLDER}")
  endif()
endif()

# this is an empty library for linking purpose only
add_library(vgg_libnode STATIC empty.cpp)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND VGG_VAR_TARGET MATCHES "^Linux")
  target_link_libraries(vgg_libnode INTERFACE atomic)
endif()
target_include_directories(vgg_libnode PUBLIC
  ${NODE_FOLDER}/src
  ${NODE_FOLDER}/deps/uv/include
  ${NODE_FOLDER}/deps/v8/include)

if(NOT VGG_VAR_TARGET MATCHES "^iOS")
  if(NOT MSVC)
    add_dependencies(vgg_libnode vgg_libnode_building)
  endif()
endif()

# this is a stub library for providing missing interface that must be linked to libnode
add_library(vgg_libnode_stub STATIC
  ${NODE_FOLDER}/src/node_snapshot_stub.cc
)
target_include_directories(vgg_libnode_stub PUBLIC
  ${NODE_FOLDER}/src
  ${NODE_FOLDER}/deps/uv/include
  ${NODE_FOLDER}/deps/v8/include)

set(LIBNODE_DEPS
  ada
  base64
  brotli
  cares
  histogram
  icudata
  icui18n
  icuucx
  llhttp
  nghttp2
  #nghttp3
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
)
if(VGG_VAR_TARGET MATCHES "^iOS")
  target_link_libraries(vgg_libnode PRIVATE
    vgg_libnode_stub
  )
elseif(APPLE)
  target_link_directories(vgg_libnode PUBLIC ${NODE_FOLDER}/out/Release)
  if (VGG_VAR_TARGET_ARCH STREQUAL "ARM")
    target_link_libraries(vgg_libnode PRIVATE
      ${LIBNODE_DEPS}
      nghttp3
      icutools
      v8_init
      base64_neon64
    )
  else()
    target_link_libraries(vgg_libnode PRIVATE
      ${LIBNODE_DEPS}
      nghttp3
      icutools
      v8_init
      base64_avx
      base64_avx2
      base64_sse41
      base64_sse42
      base64_ssse3
    )
  endif()
elseif(MSVC)
  target_link_directories(vgg_libnode PUBLIC
    ${NODE_FOLDER}/out/${CMAKE_BUILD_TYPE}
  )
  target_link_libraries(vgg_libnode PRIVATE node)
else()
  target_link_directories(vgg_libnode PUBLIC
    ${NODE_FOLDER}/out/Release/obj
    ${NODE_FOLDER}/out/Release/obj/deps/ada
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
  if (VGG_VAR_TARGET_ARCH STREQUAL "ARM")
    #message(FATAL_ERROR "Not yet support ARM platforms")
    target_link_libraries(vgg_libnode PRIVATE
      "-Wl,--whole-archive"
      ${LIBNODE_DEPS}
      base64_neon64
      "-Wl,--no-whole-archive"
    )
  elseif (VGG_VAR_TARGET_ARCH STREQUAL "RISCV")
    target_link_libraries(vgg_libnode PRIVATE
      "-Wl,--whole-archive"
      ${LIBNODE_DEPS}
      "-Wl,--no-whole-archive"
    )
  else()
    target_link_libraries(vgg_libnode PRIVATE
      "-Wl,--whole-archive"
      ${LIBNODE_DEPS}
      nghttp3
      base64_avx
      base64_avx2
      base64_avx512
      base64_sse41
      base64_sse42
      base64_ssse3
      "-Wl,--no-whole-archive"
    )
  endif()
endif()

install(TARGETS vgg_libnode_stub DESTINATION lib)
if(VGG_CONTAINER_FOR_QT)
  if(NOT MSVC)
    install(TARGETS zlib
      COMPONENT container
      DESTINATION lib
    )
  endif()
endif()
