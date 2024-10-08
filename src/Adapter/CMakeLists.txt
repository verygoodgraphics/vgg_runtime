add_library(vgg_adapter OBJECT
  PlatformAdapter/Helper/StringHelper.cpp
)

set_target_properties(vgg_adapter PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (EMSCRIPTEN)
  target_sources(vgg_adapter PRIVATE
    PlatformAdapter/Browser/Exec/BrowserJSEngine.cpp
    PlatformAdapter/Browser/Sdk/BrowserSdkAdapter.cpp
  )
else()
  if(VGG_VAR_TARGET MATCHES "^iOS")
    target_sources(vgg_adapter PRIVATE
      Container/MetalGraphicsContext.mm
    )
    target_compile_options(vgg_adapter PRIVATE
      -fno-objc-arc
    )
    target_include_directories(vgg_adapter PUBLIC
      ${CMAKE_SOURCE_DIR}/include/Application/Event
    )
  elseif(VGG_VAR_TARGET MATCHES "^Android")
    target_sources(vgg_adapter PRIVATE
      Container/AndroidGraphicsContext.cpp
    )
    target_include_directories(vgg_adapter PUBLIC
      ${CMAKE_SOURCE_DIR}/include/Application/Event
    )
  elseif(VGG_CONTAINER_FOR_QT)
    target_sources(vgg_adapter PRIVATE
      Container/QtGraphicsContext.cpp
      Container/QtQuickGraphicsContext.cpp
    )
    target_include_directories(vgg_adapter PUBLIC
      ${CMAKE_SOURCE_DIR}/include/Application/Event
    )
  endif()

  target_link_libraries(vgg_adapter vgg_libnode)
  target_sources(vgg_adapter PRIVATE
    PlatformAdapter/Native/Composer/NativeComposer.cpp
    PlatformAdapter/Native/Exec/NativeExec.cpp
    PlatformAdapter/Native/Exec/NativeExecImpl.cpp
    PlatformAdapter/Native/Sdk/Event/KeyboardEvent.cpp
    PlatformAdapter/Native/Sdk/Event/MouseEvent.cpp
    PlatformAdapter/Native/Sdk/Event/TouchEvent.cpp
    PlatformAdapter/Native/Sdk/AdapterHelper.cpp
    PlatformAdapter/Native/Sdk/VggSdkAddon.cpp
    PlatformAdapter/Native/Sdk/VggSdkNodeAdapter.cpp
    PlatformAdapter/Native/Environment.cpp
  )
  target_compile_definitions(vgg_adapter PRIVATE NODE_WANT_INTERNALS=1)
endif()

target_include_directories(vgg_adapter PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(vgg_adapter PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include/Adapter
)

target_link_libraries(vgg_adapter
  vgg_flags
  vgg_application
)
