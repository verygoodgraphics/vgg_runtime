add_library(vgg_domain STATIC
  Exec/VggExec.cpp
  Layout/AutoLayout.cpp
  Layout/BezierPoint.cpp
  Layout/ExpandSymbol.cpp
  Layout/Helper.cpp
  Layout/Layout.cpp
  Layout/LayoutNode.cpp
  Layout/Math.cpp
  Layout/Rect.cpp
  Layout/Rule.cpp
  Loader/ZipLoader.cpp
  Model/Daruma.cpp
  Model/DarumaContainer.cpp
  Model/DarumaImpl.cpp
  Model/DesignDocAdapter.cpp
  Model/DesignModel.cpp
  Model/Element.cpp
  Model/JsonDocument.cpp
  Model/JsonSchemaValidator.cpp
  Model/SchemaValidJsonDocument.cpp
  Model/SubjectJsonDocument.cpp
  Saver/DirSaver.cpp
  Saver/ZipSaver.cpp
  Saver/ZipStreamSaver.cpp
)

if(MSVC)
  # for M_PI_4
  target_compile_definitions(vgg_domain PRIVATE _USE_MATH_DEFINES)
endif()

target_include_directories(vgg_domain PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_RXCPP_INCLUDE}
  ${VGG_CONTRIB_VALIJSON_INCLUDE}
)
target_include_directories(vgg_domain PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Layout
  ${CMAKE_CURRENT_SOURCE_DIR}/Model
  ${CMAKE_SOURCE_DIR}/include/Domain
  ${CMAKE_SOURCE_DIR}/include/Domain/Layout
  ${CMAKE_SOURCE_DIR}/include/Domain/Model
  ${VGG_CONTRIB_BEZIER_INCLUDE}
  ${VGG_CONTRIB_BOOST_INCLUDE}
  ${VGG_CONTRIB_GLM_INCLUDE}
)
target_link_libraries(vgg_domain
  flexbox
  grid_layout
  vgg_flags
  vgg_utility
  zip
)
set_target_properties(vgg_domain PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS vgg_domain DESTINATION lib COMPONENT vgg_module_domain)
