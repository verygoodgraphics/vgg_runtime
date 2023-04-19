#pragma once

#include "JsonDocument.hpp"
#include "JsonSchemaValidator.hpp"

#include <memory>

class SchemaValidJsonDocument : public JsonDocument
{
public:
  using ValidatorPtr = std::shared_ptr<JsonSchemaValidator>;
  SchemaValidJsonDocument(const JsonDocumentPtr& jsonDoc,
                          const ValidatorPtr& schemaValidator = ValidatorPtr());

  void addAt(const json::json_pointer& path, const json& value) override;
  void replaceAt(const json::json_pointer& path, const json& value) override;
  void deleteAt(const json::json_pointer& path) override;

private:
  ValidatorPtr m_validator;

  void editTemplate(
    const json::json_pointer& path,
    const json& value,
    std::function<void(json&, json::json_pointer&, const json&)> tryEditFn,
    std::function<void(JsonDocumentPtr&, const json::json_pointer&, const json&)> editFn);
  bool validateDocument(const json& document);
  const json::json_pointer getNearestHavingClassAncestorPath(
    const json::json_pointer& editPath) const;
  void calculateRelativePath(const json::json_pointer& ancestorPath,
                             const json::json_pointer& currentPath,
                             json::json_pointer& relativePath);
};
