if(VGG_VAR_PLATFORM_TARGET STREQUAL "iOS")
  set(PLATFORM "OS64")
elseif(VGG_VAR_PLATFORM_TARGET STREQUAL "iOS-simulator")
  set(PLATFORM "iOS-simulator")
endif()

set(DEPLOYMENT_TARGET "15.0")
set(ENABLE_BITCODE OFF)

include(${CMAKE_CURRENT_LIST_DIR}/ios.toolchain.cmake)