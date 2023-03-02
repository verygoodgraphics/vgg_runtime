#include <gtest/gtest.h>
#include <fstream>
#include "JsonSchemaValidator.hpp"

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;

void validate(JsonSchemaValidator& sut,
              const std::string& schema_file_name,
              const std::string& json_file_name);

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

TEST_F(VggJsonSchemaValitatorTestSuite, smoke)
{
  // Given
  // Copy from https://github.com/pboettch/json-schema-validator/tree/2.2.0
  json person_schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "A person",
    "properties": {
        "name": {
            "description": "Name",
            "type": "string"
        },
        "age": {
            "description": "Age of the person",
            "type": "number",
            "minimum": 2,
            "maximum": 200
        }
    },
    "required": [
                 "name",
                 "age"
                 ],
    "type": "object"
}

)"_json;

  json bad_person = { { "age", 42 } };
  json good_person = { { "name", "Albert" }, { "age", 42 } };

  sut.setRootSchema(person_schema);

  // When && Then
  EXPECT_EQ(false, sut.validate(bad_person));
  EXPECT_EQ(true, sut.validate(good_person));
}

TEST_F(VggJsonSchemaValitatorTestSuite, withFileName)
{
  validate(sut, "./testDataDir/vgg-format.json", "./testDataDir/2020.json");
}

void validate(JsonSchemaValidator& sut,
              const std::string& schema_file_name,
              const std::string& json_file_name)
{
  // Given
  std::ifstream schema_fs(schema_file_name);
  std::ifstream json_fs(json_file_name);

  json schema = json::parse(schema_fs);
  json json_data = json::parse(json_fs);

  sut.setRootSchema(schema);

  // When && Then
  EXPECT_EQ(true, sut.validate(json_data));
}