#include <gtest/gtest.h>
#include <fstream>
#include "JsonSchemaValidator.hpp"

using json = nlohmann::json;

void validate_by_filename(JsonSchemaValidator& sut,
                          const std::string& schema_file_name,
                          const std::string& json_file_name,
                          const bool expect_result);

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
};

TEST_F(VggJsonSchemaValitatorTestSuite, GoodCase)
{
  validate_by_filename(sut, "./testDataDir/vgg-format.json", "./testDataDir/2020.json", true);
}

TEST_F(VggJsonSchemaValitatorTestSuite, BadTargetJson)
{
  validate_by_filename(sut, "./testDataDir/vgg-format.json", "./testDataDir/2020_bad.json", false);
}

void validate_by_filename(JsonSchemaValidator& sut,
                          const std::string& schema_file_name,
                          const std::string& json_file_name,
                          const bool expect_result)
{
  // Given
  std::ifstream schema_fs(schema_file_name);
  std::ifstream json_fs(json_file_name);

  json schema = json::parse(schema_fs);
  json json_data = json::parse(json_fs);

  sut.setRootSchema(schema);

  // When && Then
  EXPECT_EQ(expect_result, sut.validate(json_data));
}