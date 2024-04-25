find_package(Vulkan)
include(GetGitRevisionDescription)
if(Vulkan_FOUND)
  get_git_head_revision(REF_VAR HASH_VAR)
  message(STATUS "${REF_VAR} ${HASH_VAR}")
  add_library(vgg_exporter Exporter.cpp)
  target_compile_definitions(vgg_exporter PRIVATE GIT_COMMIT="${HASH_VAR}")
  set_target_properties(vgg_exporter PROPERTIES POSITION_INDEPENDENT_CODE ON)

  find_package(Vulkan REQUIRED)
  target_link_libraries(vgg_exporter PUBLIC vgg_layer Vulkan::Vulkan vgg_domain)

  target_include_directories(vgg_exporter PUBLIC ${CMAKE_SOURCE_DIR}/include)

  install(TARGETS vgg_exporter DESTINATION lib COMPONENT vgg_module_exporter)
  install(DIRECTORY 
    ${CMAKE_SOURCE_DIR}/include/Layer
    DESTINATION include COMPONENT vgg_module_exporter)
  install(DIRECTORY 
    ${CMAKE_SOURCE_DIR}/include/VGG/Exporter
    DESTINATION include COMPONENT vgg_module_exporter)
else()
  message("Exporter requires vulkan backend, please check your vulkan installation")
endif()
