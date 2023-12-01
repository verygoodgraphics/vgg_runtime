add_library(vgg_component STATIC
  Component.cpp
)

if(VGG_VAR_TARGET MATCHES "^iOS")
  target_sources(vgg_component PRIVATE
    MetalComponent.cpp
  )
endif()

target_include_directories(vgg_component PUBLIC
  ${CMAKE_SOURCE_DIR}/include/Application/Event
  ${CMAKE_SOURCE_DIR}/include/VGG/Component
)

target_link_libraries(vgg_component PUBLIC
  vgg_application
  vgg_adapter
)

install(TARGETS vgg_component DESTINATION lib)
if(VGG_VAR_TARGET MATCHES "^iOS")
  install(FILES
    ${CMAKE_SOURCE_DIR}/include/Application/Event/Event.hpp
    ${CMAKE_SOURCE_DIR}/include/Application/Event/Keycode.hpp
    ${CMAKE_SOURCE_DIR}/include/Application/Event/Scancode.hpp
    ${CMAKE_SOURCE_DIR}/include/VGG/Component/MetalComponent.hpp
    ${CMAKE_SOURCE_DIR}/include/VGG/Component/VggTypes.hpp
    DESTINATION VGG)
endif()
