set(MODULE_NAME layer)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Core CoreSrcVar)
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaBackend SkiaImplSrcVar)
set(SkiaImplSrcVar "${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaBackend/SkiaImpl.cpp")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Scene SceneSrcVar)
if(LAYER_SHARED_LIBRARY)
  add_library(layer SHARED)
  target_compile_definitions(layer PUBLIC -DLAYER_SHARED_LIBRARY)
else()
  add_library(layer STATIC)
endif()

set(LeagacySrcVar
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/Entity/EntityManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/Systems/GeometrySystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/Systems/RenderSystem.cpp"
    "${CMAKE_SOURCE_DIR}/src/Utils/EntityContainer.cpp")
target_sources(layer PRIVATE ${CoreSrcVar} ${SkiaImplSrcVar} ${SceneSrcVar}
                             ${LeagacySrcVar})
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)
target_include_directories(
  ${MODULE_NAME} PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${SDL2_INCLUDE_DIRS}/.. ${SKIA_INCLUDE_DIRS})

  message(STATUS "SKIA_INCLUDE_DIRS" ${SKIA_INCLUDE_DIRS})

list(APPEND COMMON_DEPS "glm")
if (NOT EMSCRIPTEN)
  list(APPEND COMMON_DEPS "vgg_sketch_parser")
endif()

target_link_libraries(layer PUBLIC ${COMMON_DEPS})

if (EMSCRIPTEN)
  set_target_properties(layer PROPERTIES
    COMPILE_FLAGS "-DSK_GL -s USE_SDL=2 -fexceptions"
  )
elseif (APPLE)
  set_target_properties(layer PROPERTIES
    COMPILE_FLAGS "-DSK_GL -I${SDL2_INCLUDE_DIRS}/.."
  )
endif()
