#pragma once

#include "JsonSchemaValidator.hpp"

#include "gmock/gmock.h"

class MockJsonSchemaValidator : public JsonSchemaValidator
{
public:
  // MOCK_METHOD(const std::string, documentJson, (), (const));
  // MOCK_METHOD(const std::string, findElement, (const std::string&), (const));
  // MOCK_METHOD(void, updateStyle, (), ());
};
