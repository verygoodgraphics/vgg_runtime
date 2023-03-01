#include <gtest/gtest.h>

#include "JsonSchemaValidator.hpp"

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;

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