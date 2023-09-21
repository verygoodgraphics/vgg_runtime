include(ImportSkia)
file(GLOB SrcVar *.cpp)
if(LAYER_SHARED_LIBRARY)
  add_library(vgg_layer SHARED)
  target_compile_definitions(vgg_layer PUBLIC -DLAYER_SHARED_LIBRARY)
else()
  add_library(vgg_layer STATIC)
endif()

option(ENABLE_EMBBED_FONT "Enable embbed FiraSans font" ON)
mark_as_advanced(ENABLE_EMBBED_FONT)

if(ENABLE_EMBBED_FONT)
  target_compile_definitions(vgg_layer PRIVATE VGG_USE_EMBBED_FONT)
  target_link_libraries(vgg_layer PRIVATE zip)
  install(TARGETS zip DESTINATION lib COMPONENT vgg_module_zip)
endif()

target_sources(vgg_layer PRIVATE ${SrcVar})
target_include_directories(vgg_layer PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_include_directories(
  vgg_layer PRIVATE
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_RAPIDFUZZCPP_INCLUDE})

# find OpenGL library
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

target_include_directories(vgg_layer PUBLIC ${VGG_CONTRIB_JSON_INCLUDE})
# REFACTOR:: make skia hidden from public
target_link_libraries(vgg_layer PUBLIC ${SKIA_LIBS} glm)
target_link_libraries(vgg_layer PRIVATE ${OPENGL_LIBRARIES} vgg_utility)
find_package(Vulkan)
if(Vulkan_FOUND)
  message("Vulkan found: enabling vulkan ...")
  target_compile_definitions(vgg_layer PRIVATE VGG_USE_VULKAN)
  target_link_libraries(vgg_layer PRIVATE Vulkan::Vulkan)
endif()

set_target_properties(vgg_layer PROPERTIES BUILD_RPATH ${SKIA_LIB_DIR})
set_target_properties(vgg_layer PROPERTIES POSITION_INDEPENDENT_CODE ON)

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
