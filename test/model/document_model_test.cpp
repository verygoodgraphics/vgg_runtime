#include "DocumentModel.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <iostream>

using namespace nlohmann;

#define JSON_SCHEMA_FILE_NAME "./asset/vgg-format.json"
#define DOCUMENT_FILE_NAME "./testDataDir/1_out_json.json"

class DocumentModelTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<DocumentModel> sut;

  void SetUp() override
  {
    std::ifstream schema_fs(JSON_SCHEMA_FILE_NAME);
    json schema = json::parse(schema_fs);

    std::ifstream document_fs(DOCUMENT_FILE_NAME);
    json document = json::parse(document_fs);

    auto schema_validator = std::make_shared<JsonSchemaValidator>();
    schema_validator->setRootSchema(schema);

    sut.reset(new DocumentModel(schema_validator, document));
  }

  void TearDown() override
  {
  }
};

TEST_F(DocumentModelTestSuite, Smoke)
{
  EXPECT_TRUE(sut);
}

TEST_F(DocumentModelTestSuite, Add_ChangeColorToInvalidValue)
{
  // Given
  const auto path =
    "/artboard/0/layers/0/childObjects/0/style/borders/0/color/badfield"_json_pointer;

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

TEST_F(DocumentModelTestSuite, Replace_ValidValue)
{
  const auto path = "/artboard/0/layers/0/childObjects/0/style/borders/0/color/alpha"_json_pointer;
  const auto value = 0.1;
  sut->replaceAt(path, value);

  const auto document = sut->documentJson();
  const auto new_value = document[path];

  EXPECT_EQ(new_value, value);
}

TEST_F(DocumentModelTestSuite, Replace_Filetype)
{
  const auto path = "/fileType"_json_pointer;
  const auto value = 1;
  sut->replaceAt(path, value);

  const auto document = sut->documentJson();
  const auto new_value = document[path];

  EXPECT_EQ(new_value, value);
}

TEST_F(DocumentModelTestSuite, Delete_ChangeColorToInvalidValue)
{
  // Given
  const auto path = "/artboard/0/layers/0/childObjects/0/style/borders/0/color/alpha"_json_pointer;

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
