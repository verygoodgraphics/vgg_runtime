add_library(vgg_adapter OBJECT
  PlatformAdapter/Helper/StringHelper.cpp
  View/UIView.cpp
)

if (EMSCRIPTEN)
  target_sources(vgg_adapter PRIVATE
    PlatformAdapter/Browser/Exec/BrowserJSEngine.cpp
    PlatformAdapter/Browser/Sdk/BrowserSdkAdapter.cpp
  )
else()
  target_link_libraries(vgg_adapter vgg_libnode)
  set_target_properties(vgg_adapter PROPERTIES LINK_FLAGS ${SDL2_LIBRARIES})
  target_sources(vgg_adapter PRIVATE
    PlatformAdapter/Native/Exec/NativeExec.cpp
    PlatformAdapter/Native/Exec/NativeExecImpl.cpp
    PlatformAdapter/Native/Sdk/Event/KeyboardEvent.cpp
    PlatformAdapter/Native/Sdk/Event/MouseEvent.cpp
    PlatformAdapter/Native/Sdk/Event/TouchEvent.cpp
    PlatformAdapter/Native/Sdk/AdapterHelper.cpp
    PlatformAdapter/Native/Sdk/VggSdkAddon.cpp
    PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.cpp
  )
  target_compile_definitions(vgg_adapter PRIVATE NODE_WANT_INTERNALS=1)
endif()

target_include_directories(vgg_adapter PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(vgg_adapter PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Adapter
  ${SDL2_INCLUDE_DIRS}/..
)

target_link_libraries(vgg_adapter
  vgg_flags
  vgg_application
)
