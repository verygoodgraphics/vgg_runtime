#include "Domain/Config.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Domain/SchemaValidJsonDocument.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <iostream>

using namespace VGG::Model;
using namespace nlohmann;

#define JSON_SCHEMA_FILE_NAME "./asset/vgg-format.json"

class SchemaValidJsonDocumentTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<SchemaValidJsonDocument> sut;

  void SetUp() override
  {
    std::ifstream schema_fs(JSON_SCHEMA_FILE_NAME);
    json schema = json::parse(schema_fs);

    auto file_path = std::string{ "./testDataDir/vgg-daruma/" } + design_file_name;
    std::ifstream document_fs(file_path);
    json document = json::parse(document_fs);

    auto schema_validator = std::make_shared<JsonSchemaValidator>();
    schema_validator->setRootSchema(schema);

    auto raw_json_doc = std::make_shared<RawJsonDocument>();
    raw_json_doc->setContent(document);

    sut.reset(new SchemaValidJsonDocument(raw_json_doc, schema_validator));
  }

  void TearDown() override
  {
  }
};

TEST_F(SchemaValidJsonDocumentTestSuite, Smoke)
{
  EXPECT_TRUE(sut);
}

TEST_F(SchemaValidJsonDocumentTestSuite, Add_ChangeColorToInvalidValue)
{
  // Given
  const auto path = "/symbolMaster/0/backgroundColor/fakeProperty"_json_pointer;

  try
  {
    // When
    sut->addAt(path, "aValue");
  }
  catch (std::exception& e)
  {
    std::cout << "Test Add_ChangeColorToInvalidValue, excetion: " << e.what() << std::endl;
    // Then
    GTEST_SUCCEED();
    return;
  }

  GTEST_FAIL();
}

TEST_F(SchemaValidJsonDocumentTestSuite, Replace_ValidValue)
{
  const auto path = "/symbolMaster/0/backgroundColor/alpha"_json_pointer;
  const auto value = 0.1;
  sut->replaceAt(path, value);

  const auto document = sut->content();
  const auto new_value = document[path];

  EXPECT_EQ(new_value, value);
}

TEST_F(SchemaValidJsonDocumentTestSuite, Replace_Filetype)
{
  const auto path = "/fileType"_json_pointer;
  const auto value = 1;
  sut->replaceAt(path, value);

  const auto document = sut->content();
  const auto new_value = document[path];

  EXPECT_EQ(new_value, value);
}

TEST_F(SchemaValidJsonDocumentTestSuite, Delete_ChangeColorToInvalidValue)
{
  // Given
  const auto path = "/symbolMaster/0/backgroundColor/alpha"_json_pointer;

  try
  {
    // When
    sut->deleteAt(path);
  }
  catch (std::logic_error& e)
  {
    std::cout << "Test ChangeColorToInvalidValue, excetion: " << e.what() << std::endl;
    // Then
    GTEST_SUCCEED();
    return;
  }

  GTEST_FAIL();
}
