if (EMSCRIPTEN)
else()
  add_subdirectory(render)
  if(NOT ENABLE_UNIT_TEST)
    return()
  endif()

  add_executable(unit_tests
    container/MockSkiaGraphicsContext.cpp
    container/SdkTests.cpp
    container/container_tests.cpp
    controller/controller_test.cpp
    domain/layout/expand_symbol_tests.cpp
    domain/layout/layout_tests.cpp
    domain/layout/lib_layout_tests.cpp
    domain/layout/resizing_tests.cpp
    domain/model/DesignModelTests.cpp
    domain/model/daruma_helper.cpp
    editor/save_test.cpp
    exec/vgg_exec_test.cpp
    infrastructure/async_test.cpp
    model/automerge_test.cpp
    model/json_schema_validator_test.cpp
    model/schema_valid_json_document_test.cpp
    model/vgg_model_test.cpp
    native/native_exec_test.cpp
    native/native_sdk_test.cpp
    native/node_test.cpp
    native/node_test_helper.cpp
    usecase/start_running_tests.cpp
    layer/refcounter_test.cpp
    layer/treenode_test.cpp
    layer/observe_test.cpp
    Utility/TimerTests.cpp
  )
  target_include_directories(unit_tests PRIVATE
    .
    mocks
    model
    ${VGG_AUTOMERGE_INCLUDE}
  )

  get_target_property(VGG_LAYER_RPATH vgg_layer BUILD_RPATH)
  set_target_properties(unit_tests PROPERTIES
    BUILD_RPATH ${VGG_LAYER_RPATH})

  set(TMP_VGG_WORK_ZIP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir/vgg-daruma.zip)
  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar cvf ${TMP_VGG_WORK_ZIP_FILE} --format=zip -- .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir/vgg-daruma
  )
  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir $<TARGET_FILE_DIR:unit_tests>/testDataDir
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/asset $<TARGET_FILE_DIR:unit_tests>/asset
  )
  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm ${TMP_VGG_WORK_ZIP_FILE}
  )

  target_link_libraries(unit_tests PRIVATE vgg_internal gtest_main gmock vgg_utility automerge
    vgg_container
  )
  include(GoogleTest)
  gtest_discover_tests(unit_tests)
endif()
