set(MODULE_NAME layer)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/Core CoreSrcVar)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/SkiaBackend SkiaImplSrcVar)
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
    "${CMAKE_SOURCE_DIR}/src/Utils/EntityContainer.cpp"
    "${CMAKE_SOURCE_DIR}/src/Utils/WasmBindings.cpp")
target_sources(layer PRIVATE ${CoreSrcVar} ${SkiaImplSrcVar} ${SceneSrcVar}
                             ${LeagacySrcVar})
target_include_directories(${MODULE_NAME}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)
target_include_directories(
  ${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                         ${SDL2_INCLUDE_DIRS}/..)

list(APPEND COMMON_DEPS "glm" "vgg_sketch_parser")

target_link_libraries(layer PUBLIC ${COMMON_DEPS})
