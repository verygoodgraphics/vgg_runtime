add_library(vgg_application STATIC
  Animate.cpp
  AppLayoutContext.cpp
  AppRender.cpp
  AttrBridge.cpp
  Controller.cpp
  Editor.cpp
  ElementGetPropertySerializer.cpp
  EventAPI.cpp
  MainComposer.cpp
  Pager.cpp
  Presenter.cpp
  Reporter.cpp
  RunLoop.cpp
  UIApplication.cpp
  UIGestureRecognizer.cpp
  UIPanGestureRecognizer.cpp
  UIScrollView.cpp
  UIScrollViewAnimation.cpp
  UIScrollViewAnimationDeceleration.cpp
  UIView.cpp
  UIViewImpl.cpp
  VggEnv.cpp
  VggSdk.cpp
  ViewModel.cpp
  VGGLayer.cpp
)

if(MSVC)
  target_compile_definitions(vgg_application PRIVATE VGG_RUNTIME_DLL_EXPORTING)
endif()

target_include_directories(vgg_application PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/VGG/Container
  ${CMAKE_SOURCE_DIR}/include/Package
)

target_include_directories(vgg_application PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/include/Application
  ${CMAKE_SOURCE_DIR}/include/Adapter
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
