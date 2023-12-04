add_library(vgg_container STATIC
  Container.cpp
)

if(VGG_VAR_TARGET MATCHES "^iOS")
  target_sources(vgg_container PRIVATE
    MetalContainer.cpp
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

install(TARGETS vgg_container DESTINATION lib)
if(VGG_VAR_TARGET MATCHES "^iOS")
  install(FILES
    ${CMAKE_SOURCE_DIR}/include/Application/Event/Event.hpp
    ${CMAKE_SOURCE_DIR}/include/Application/Event/Keycode.hpp
    ${CMAKE_SOURCE_DIR}/include/Application/Event/Scancode.hpp
    ${CMAKE_SOURCE_DIR}/include/VGG/Container/MetalContainer.hpp
    ${CMAKE_SOURCE_DIR}/include/VGG/Container/VggTypes.hpp
    DESTINATION VGG)
endif()
