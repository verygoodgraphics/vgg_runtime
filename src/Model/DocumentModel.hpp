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

  const json& documentJson() const;

private:
  Automerge m_doc;
  JsonSchemaValidator m_validator;

  void editTemplate(const json::json_pointer& path,
                    const json& value,
                    std::function<void(json&, json::json_pointer&, const json&)> tryEditFn,
                    std::function<void(Automerge&, const json::json_pointer&, const json&)> editFn);
  bool validateHavingClassDocument(const json& document);
  const json::json_pointer getNearestHavingClassAncestorPath(
    const json::json_pointer& editPath) const;
  void calculateRelativePath(const json::json_pointer& ancestorPath,
                             const json::json_pointer& currentPath,
                             json::json_pointer& relativePath);
  void save();
};

#endif