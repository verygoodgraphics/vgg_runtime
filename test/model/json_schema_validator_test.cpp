#include "Domain/Config.hpp"
#include "Domain/JsonSchemaValidator.hpp"

#include <gtest/gtest.h>
#include <fstream>

using namespace VGG::Model;
using json = nlohmann::json;

#define VGG_JSON_SCHEMA_FILE_NAME "./asset/vgg-format.json"

class VggJsonSchemaValitatorTestSuite : public ::testing::Test
{
protected:
  JsonSchemaValidator sut;

  void SetUp() override
  {
  }
  void TearDown() override
  {
  }

  void setRootSchemaByFileName(const std::string& schema_file_name)
  {
    json schema = load_json(schema_file_name);
    sut.setRootSchema(schema);
  }

  json load_json(const std::string& json_file_name)
  {
    std::ifstream json_fs(json_file_name);
    json json_data = json::parse(json_fs);

    return json_data;
  }

  void validate_by_filename(const std::string& json_file_name, const bool expect_result)
  {
    // Given
    json json_data = load_json(json_file_name);

    // When && Then
    EXPECT_EQ(expect_result, sut.validate(json_data));
  }
};

TEST_F(VggJsonSchemaValitatorTestSuite, GoodCase)
{
  setRootSchemaByFileName(VGG_JSON_SCHEMA_FILE_NAME);
  validate_by_filename(std::string{ "./testDataDir/vgg-daruma/" } + design_file_name, true);
}

TEST_F(VggJsonSchemaValitatorTestSuite, BadTargetJson)
{
  setRootSchemaByFileName(VGG_JSON_SCHEMA_FILE_NAME);
  validate_by_filename("./testDataDir/2020_bad.json", false);
}

TEST_F(VggJsonSchemaValitatorTestSuite, ValidateByClassName)
{
  // Given
  setRootSchemaByFileName(VGG_JSON_SCHEMA_FILE_NAME);
  json json_data = load_json("./testDataDir/color.json");
  std::string class_name = "color";

  // When
  auto result = sut.validate(class_name, json_data);

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggJsonSchemaValitatorTestSuite, NoSchema)
{
  validate_by_filename(std::string{ "./testDataDir/vgg-daruma/" } + design_file_name, true);
}

TEST_F(VggJsonSchemaValitatorTestSuite, NoSchemaValidateByClassName)
{
  // Given
  json json_data = load_json("./testDataDir/color.json");
  std::string class_name = "color";

  // When
  auto result = sut.validate(class_name, json_data);

  // Then
  EXPECT_EQ(result, false);
}