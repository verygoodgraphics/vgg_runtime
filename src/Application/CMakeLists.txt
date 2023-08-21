add_library(vgg_application STATIC
  Controller/Controller.cpp
  Sdk/VggSdk.cpp
  src/Renderer/RenderAdapter.cpp
  src/RenderAdapter.cpp
  src/EventImpl.cpp
)

target_include_directories(vgg_application PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/interface
)

target_include_directories(vgg_application PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Application
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vgg_application PUBLIC
  vgg_flags
  vgg_usecase
  vgg_layer
)
target_link_libraries(vgg_application PRIVATE
  automerge
)

install(TARGETS vgg_application DESTINATION lib COMPONENT vgg_module_application)
