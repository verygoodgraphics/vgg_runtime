include(ImportSkia)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Core CoreSrcVar)
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaBackend SkiaImplSrcVar)
set(SkiaImplSrcVar "${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaBackend/SkiaImpl.cpp")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Scene SceneSrcVar)
if(LAYER_SHARED_LIBRARY)
  add_library(vgg_layer SHARED)
  target_compile_definitions(vgg_layer PUBLIC -DLAYER_SHARED_LIBRARY)
else()
  add_library(vgg_layer STATIC)
endif()

set(LeagacySrcVar
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/Entity/EntityManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/Systems/GeometrySystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/Systems/RenderSystem.cpp"
    "${CMAKE_SOURCE_DIR}/src/Utils/EntityContainer.cpp"
)
target_sources(vgg_layer PRIVATE ${CoreSrcVar} ${SkiaImplSrcVar} ${SceneSrcVar}
                             ${LeagacySrcVar})
target_include_directories(vgg_layer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)
target_include_directories(
  vgg_layer PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${SDL2_INCLUDE_DIRS}/..)

list(APPEND COMMON_DEPS "glm")
if (NOT EMSCRIPTEN)
  list(APPEND COMMON_DEPS "vgg_sketch_parser")
endif()

target_link_libraries(vgg_layer PUBLIC ${COMMON_DEPS} ${SKIA_LIBS})
target_link_libraries(vgg_layer PRIVATE ${OPENGL_LIBRARIES})

if(EMSCRIPTEN)
# It's temporay because layer need the SDL event type declaration.
# This will be removed after a self-contained event framework introduced
set_target_properties(vgg_layer PROPERTIES
    COMPILE_FLAGS "-s USE_SDL=2 -fexceptions"
    LINK_FLAGS "-s USE_SDL=2")
endif()
