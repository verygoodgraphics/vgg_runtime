aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} UtilitySrc)
add_library(vgg_utility)
target_sources(vgg_utility PRIVATE ${UtilitySrc})
set_target_properties(vgg_utility PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(vgg_utility PUBLIC ${CMAKE_SOURCE_DIR}/include ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_RXCPP_INCLUDE}
)
target_include_directories(vgg_utility PRIVATE ${CMAKE_SOURCE_DIR}/include/Utility)
install(TARGETS vgg_utility DESTINATION lib COMPONENT vgg_module_utility)
