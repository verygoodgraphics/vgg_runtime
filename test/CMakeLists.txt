if (EMSCRIPTEN)
else()
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(unit_tests
    model/json_schema_validator_test.cpp
  )
  target_include_directories(unit_tests PRIVATE
    model 
  )

  add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/testDataDir $<TARGET_FILE_DIR:unit_tests>/testDataDir)

  target_link_libraries(unit_tests PRIVATE vgg_internal gtest_main)
  include(GoogleTest)
  gtest_discover_tests(unit_tests)
endif()