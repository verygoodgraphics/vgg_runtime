#pragma once

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class DocumentStore
{
public:
  virtual ~DocumentStore() = default;

  virtual void setDocument(const nlohmann::json& document) = 0;
  virtual const json& document() const = 0;

  virtual void addAt(const json::json_pointer& path, const json& value) = 0;
  virtual void replaceAt(const json::json_pointer& path, const json& value) = 0;
  virtual void deleteAt(const json::json_pointer& path) = 0;
};
