set(MODULE_NAME layer)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Core CoreSrcVar)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaBackend SkiaImplSrcVar)
add_library(layer ${CoreSrcVar} ${SkiaImplSrcVar})
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(layer PRIVATE skia glm vgg_sketch_parser)
