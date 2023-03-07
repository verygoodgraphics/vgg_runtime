#ifndef _DOCUMENT_MODEL_HPP_
#define _DOCUMENT_MODEL_HPP_

#include "Automerge.h"
#include "JsonSchemaValidator.hpp"

class DocumentModel
{
public:
  DocumentModel(const nlohmann::json& schema, const nlohmann::json& document);
  ~DocumentModel();

  void addAt(const json::json_pointer& path, const json& value);
  void replaceAt(const json::json_pointer& path, const json& value);
  void deleteAt(const json::json_pointer& path);

  json documentJson();

private:
  Automerge doc_;
  JsonSchemaValidator validator_;

  void validate();
  void save();
};

#endif