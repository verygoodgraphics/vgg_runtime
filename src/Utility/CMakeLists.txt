aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src UtilitySrc)
add_library(vgg_utility INTERFACE)
# target_sources(vgg_utility PRIVATE ${UtilitySrc})
target_include_directories(vgg_utility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/interface)
