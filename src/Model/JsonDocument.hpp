
#pragma once

#include "nlohmann/json.hpp"

#include <memory>

class JsonDocument;
using JsonDocumentPtr = std::shared_ptr<JsonDocument>;
using json = nlohmann::json;

class JsonDocument
{
public:
  JsonDocument(JsonDocumentPtr jsonDoc = JsonDocumentPtr())
    : m_jsonDoc(jsonDoc)
  {
  }
  virtual ~JsonDocument() = default;

  virtual const json& content() const = 0;
  virtual void setContent(const json& document) = 0;

  virtual void addAt(const json::json_pointer& path, const json& value) = 0;
  virtual void replaceAt(const json::json_pointer& path, const json& value) = 0;
  virtual void deleteAt(const json::json_pointer& path) = 0;

  virtual void undo()
  {
  }
  virtual void redo()
  {
  }
  virtual void save()
  {
  }

protected:
  JsonDocumentPtr m_jsonDoc;
};
