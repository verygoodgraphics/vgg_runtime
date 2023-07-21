include(ImportSkia)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Core CoreSrcVar)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaImpl SkiaImplSrcVar)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Scene SceneSrcVar)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Reader ReaderSrcVar)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Common CommonSrcVar)
if(LAYER_SHARED_LIBRARY)
  add_library(vgg_layer SHARED)
  target_compile_definitions(vgg_layer PUBLIC -DLAYER_SHARED_LIBRARY)
else()
  add_library(vgg_layer STATIC)
endif()

target_sources(vgg_layer PRIVATE ${CoreSrcVar} ${SkiaImplSrcVar} ${SceneSrcVar} ${ReaderSrcVar} ${CommonSrcVar})
target_include_directories(vgg_layer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_include_directories(
  vgg_layer PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${SDL2_INCLUDE_DIRS}/..
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_RAPIDFUZZCPP_INCLUDE})

target_link_libraries(vgg_layer PUBLIC glm ${SKIA_LIBS})
target_link_libraries(vgg_layer PRIVATE ${OPENGL_LIBRARIES} vgg_utility)
set_target_properties(vgg_layer PROPERTIES BUILD_RPATH ${SKIA_LIB_DIR})

set_target_properties(vgg_layer PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(EMSCRIPTEN)
# It's temporay because layer need the SDL event type declaration.
# This will be removed after a self-contained event framework introduced
set_target_properties(vgg_layer PROPERTIES
    COMPILE_FLAGS "-s USE_SDL=2 -fexceptions"
    LINK_FLAGS "-s USE_SDL=2")
endif()

# TODO: install(IMPORTED_RUNTIME_ARTIFACTS ...) maybe a better choice
set(SKIA_ARTIFACTS_LOCATION)
foreach(SKIA_TARGET ${SKIA_LIBS})
  unset(location CACHE)
  get_target_property(location ${SKIA_TARGET} IMPORTED_LOCATION)
  list(APPEND SKIA_ARTIFACTS_LOCATION ${location})
endforeach()
if(SKIA_ARTIFACTS_LOCATION)
  install(FILES ${SKIA_ARTIFACTS_LOCATION} DESTINATION lib COMPONENT vgg_module_layer)
endif()
install(TARGETS vgg_layer ARCHIVE DESTINATION lib COMPONENT vgg_module_layer)
