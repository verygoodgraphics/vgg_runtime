add_library(vgg_application STATIC
  Controller/Controller.cpp
)

target_include_directories(vgg_application PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(vgg_application PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Application
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vgg_application PUBLIC
  vgg_domain
  vgg_layer
)