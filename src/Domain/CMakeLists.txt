add_library(vgg_domain STATIC
  Exec/VggExec.cpp
  Loader/ZipLoader.cpp
  Model/Daruma.cpp
  Model/DarumaContainer.cpp
  Model/JsonDocument.cpp
  Model/JsonSchemaValidator.cpp
  Model/SchemaValidJsonDocument.cpp
  Model/SubjectJsonDocument.cpp
  Saver/DirSaver.cpp
  Saver/ZipSaver.cpp
  Sdk/VggSdk.cpp
)

target_include_directories(vgg_domain PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${VGG_CONTRIB_JSON_INCLUDE}
  ${VGG_CONTRIB_RXCPP_INCLUDE}
  ${VGG_CONTRIB_VALIJSON_INCLUDE}
)
target_include_directories(vgg_domain PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Domain
  ${VGG_CONTRIB_BOOST_INCLUDE}
)
target_link_libraries(vgg_domain
  vgg_flags
  vgg_utility
  zip
)

install(TARGETS vgg_domain DESTINATION COMPONENT vgg_module_domain)
