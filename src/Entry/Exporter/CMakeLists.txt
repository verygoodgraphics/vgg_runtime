find_package(Vulkan)
if(Vulkan_FOUND AND ${VGG_VAR_PLATFORM_TARGET} IN_LIST VGG_LINUX_TARGET_LIST)
  add_library(vgg_exporter src/ImageExporter.cpp)
  set_target_properties(vgg_exporter PROPERTIES POSITION_INDEPENDENT_CODE ON)

  find_package(Vulkan REQUIRED)
  target_link_libraries(vgg_exporter PUBLIC vgg_layer Vulkan::Vulkan vgg_domain)


  target_include_directories(vgg_exporter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_include_directories(vgg_exporter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

  install(TARGETS vgg_exporter DESTINATION lib COMPONENT vgg_module_exporter)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/interface/ImageExporter.hpp DESTINATION include COMPONENT vgg_module_exporter)
else()
  message("Exporter requires vulkan backend, please check your vulkan installation")
endif()
