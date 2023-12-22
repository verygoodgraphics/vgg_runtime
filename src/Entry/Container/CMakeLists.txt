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

target_link_libraries(vgg_container PUBLIC
  vgg_application
  vgg_adapter
)

install(TARGETS vgg_container
  COMPONENT container
  DESTINATION lib
)
install(FILES
  ${CMAKE_SOURCE_DIR}/include/Application/Event/Event.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/EventAPI.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/Keycode.hpp
  ${CMAKE_SOURCE_DIR}/include/Application/Event/Scancode.hpp
  ${CMAKE_SOURCE_DIR}/include/VGG/Container/IContainer.hpp
  ${CMAKE_SOURCE_DIR}/include/VGG/Container/ISdk.hpp
  ${CMAKE_SOURCE_DIR}/include/VGG/Container/VggTypes.hpp
  COMPONENT container
  DESTINATION VGG)
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
