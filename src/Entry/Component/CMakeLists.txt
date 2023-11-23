add_library(vgg_component STATIC
  Component.cpp
  MetalComponent.cpp
)

target_include_directories(vgg_component PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(vgg_component PRIVATE
  ${CMAKE_SOURCE_DIR}/include/Component
)

target_link_libraries(vgg_component PUBLIC
  vgg_application
  vgg_adapter
)
