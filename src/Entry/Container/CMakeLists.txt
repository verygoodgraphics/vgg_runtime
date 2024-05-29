if(VGG_CONTAINER_FOR_QT)
  add_library(vgg_container SHARED
    Container.cpp
  )
else()
  add_library(vgg_container STATIC
    Container.cpp
  )
endif()

if(VGG_VAR_TARGET MATCHES "^iOS")
  target_sources(vgg_container PRIVATE
    MetalContainer.cpp
  )
elseif(VGG_CONTAINER_FOR_QT)
  target_sources(vgg_container PRIVATE
    QtContainer.cpp
  )
endif()

target_include_directories(vgg_container PUBLIC
  ${CMAKE_SOURCE_DIR}/include/Application/Event
  ${CMAKE_SOURCE_DIR}/include/VGG/Container
)

if(MSVC)
  target_link_libraries(vgg_container PUBLIC
    vgg_application
    vgg_adapter
  )
  target_compile_definitions(vgg_container PRIVATE VGG_RUNTIME_DLL_EXPORTING)
  target_link_options(vgg_container PRIVATE /WHOLEARCHIVE:vgg_application)
elseif(APPLE)
  target_link_libraries(vgg_container PUBLIC
    -force_load
    vgg_application
    vgg_adapter
  )
else()
  target_link_libraries(vgg_container PUBLIC
    # "-Wl,--whole-archive"
    # vgg_application
    # "-Wl,--no-whole-archive"
    vgg_adapter
  )
endif()

install(TARGETS vgg_container
  COMPONENT container
  DESTINATION lib
)
install(FILES
  ${CMAKE_SOURCE_DIR}/include/Adapter/Environment.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/Event.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/EventAPI.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/Keycode.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/Scancode.hpp
  ${CMAKE_SOURCE_DIR}/include/Package/VggPackage.hpp
  ${CMAKE_SOURCE_DIR}/include/VGG/Container/IContainer.hpp
  ${CMAKE_SOURCE_DIR}/include/VGG/Container/ISdk.hpp
  ${CMAKE_SOURCE_DIR}/include/VGG/Container/VggTypes.hpp
  COMPONENT container
  DESTINATION VGG)
install(FILES
  ${CMAKE_SOURCE_DIR}/include/Application/UIAnimation.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/UIOptions.hpp
  COMPONENT container
  DESTINATION VGG/Application)

if(MSVC)
  install(FILES
    "${CMAKE_SOURCE_DIR}/lib/${NODE_FOLDER}/${CMAKE_BUILD_TYPE}/node.exe"
    "${CMAKE_SOURCE_DIR}/lib/${NODE_FOLDER}/${CMAKE_BUILD_TYPE}/libnode.dll"
    "${SKIA_EXTERNAL_PROJECT_DIR}/${SKIA_LIB_BUILD_PREFIX}/icudtl.dat"
    "${SKIA_LIB_DIR}/skia.dll"
    "${SKIA_LIB_DIR}/skparagraph.dll"
    "${SKIA_LIB_DIR}/skshaper.dll"
    "${SKIA_LIB_DIR}/sktext.dll"
    "${SKIA_LIB_DIR}/skunicode.dll"
    "${SDL2_DLL_PATH}"
    COMPONENT container
    DESTINATION lib
  )
endif()

if(VGG_VAR_TARGET MATCHES "^iOS")
  install(FILES
    ${CMAKE_SOURCE_DIR}/include/VGG/Container/MetalContainer.hpp
    COMPONENT container
    DESTINATION VGG)
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/nodejs-mobile-ios/NodeMobile.xcframework
    COMPONENT container
    DESTINATION .)
elseif(VGG_CONTAINER_FOR_QT)
  install(FILES
    ${CMAKE_SOURCE_DIR}/include/VGG/Container/QtContainer.hpp
    COMPONENT container
    DESTINATION VGG
  )
endif()
