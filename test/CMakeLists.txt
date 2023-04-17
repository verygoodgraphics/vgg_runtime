if (EMSCRIPTEN)
else()
  option(ENABLE_UNIT_TEST "Enable unit test" OFF)
  if(NOT ENABLE_UNIT_TEST)
    message(STATUS "Unit test is disabled. Enable it with option if needed: cmake -DENABLE_UNIT_TEST=ON")
    return()
  endif()

  include(FetchContent)
  message(STATUS "Downloading gtest...")
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(unit_tests
    controller/controller_test.cpp
    exec/vgg_exec_test.cpp
    model/automerge_test.cpp
    model/design_document_model_test.cpp
    model/document_model_test.cpp
    model/json_schema_validator_test.cpp
    model/schema_valid_json_document_test.cpp
    model/vgg_work_test.cpp
    native/native_exec_test.cpp
    native/native_sdk_test.cpp
    native/node_test.cpp
    native/node_test_helper.cpp
    sdk/vgg_sdk_test.cpp
  )
  target_include_directories(unit_tests PRIVATE
    mocks
    model
  )

  set(TMP_VGG_WORK_ZIP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir/vgg-work.zip)
  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar cvf ${TMP_VGG_WORK_ZIP_FILE} --format=zip -- .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir/vgg-work
  )
  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir $<TARGET_FILE_DIR:unit_tests>/testDataDir
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/asset $<TARGET_FILE_DIR:unit_tests>/asset
  )
  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm ${TMP_VGG_WORK_ZIP_FILE}
  )

  target_link_libraries(unit_tests PRIVATE vgg_internal gtest_main gmock)
  include(GoogleTest)
  gtest_discover_tests(unit_tests)
endif()