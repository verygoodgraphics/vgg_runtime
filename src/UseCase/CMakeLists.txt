add_library(vgg_usecase STATIC
  InstanceState.cpp
  ModelChanged.cpp
  ResizeWindow.cpp
  StartRunning.cpp
)

target_include_directories(vgg_usecase PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(vgg_usecase PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include/UseCase
)

target_link_libraries(vgg_usecase PUBLIC
  vgg_flags
  vgg_domain
)

install(TARGETS vgg_usecase DESTINATION lib COMPONENT vgg_module_usecase)
