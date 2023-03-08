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

  json documentJson() const;

private:
  Automerge m_doc;
  JsonSchemaValidator m_validator;

  bool validate(const json& document, const json::json_pointer& path);
  void save();
};

#endif