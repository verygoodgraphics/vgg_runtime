add_library(vgg_application STATIC
  AppRender.cpp
  Controller.cpp
  Editor.cpp
  EventAPI.cpp
  MainComposer.cpp
  Presenter.cpp
  Reporter.cpp
  UIApplication.cpp
  UIView.cpp
  VggSdk.cpp
)

target_include_directories(vgg_application PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(vgg_application PRIVATE
  ${CMAKE_SOURCE_DIR}/include/Application
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
