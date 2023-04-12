#pragma once

#include "DocumentModel.hpp"

#include "gmock/gmock.h"

class MockDocumentModel : public DocumentModel
{
public:
  MockDocumentModel(std::shared_ptr<JsonSchemaValidator> schemaValidator,
                    const nlohmann::json& document)
    : DocumentModel(schemaValidator, document)
  {
  }

  // MOCK_METHOD(const std::string, documentJson, (), (const));
  // MOCK_METHOD(const std::string, findElement, (const std::string&), (const));
  // MOCK_METHOD(void, updateStyle, (), ());
};
