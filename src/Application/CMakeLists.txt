set(VGG_APPLICATION_SOURCE
  AppRender.cpp
  Controller.cpp
  Editor.cpp
  EventAPI.cpp
  MainComposer.cpp
  Presenter.cpp
  Reporter.cpp
  RunLoop.cpp
  UIApplication.cpp
  UIView.cpp
  VggEnv.cpp
  VggSdk.cpp
  ViewModel.cpp
)

if(MSVC)
  add_library(vgg_application SHARED ${VGG_APPLICATION_SOURCE})
  set_target_properties(vgg_application PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
  add_library(vgg_application STATIC ${VGG_APPLICATION_SOURCE})
endif()

target_include_directories(vgg_application PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/VGG/Container
)

target_include_directories(vgg_application PRIVATE
  ${CMAKE_BINARY_DIR}
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
