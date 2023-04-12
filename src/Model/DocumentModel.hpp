#ifndef _DOCUMENT_MODEL_HPP_
#define _DOCUMENT_MODEL_HPP_

#include "JsonDocumentStore.hpp"
#include "JsonSchemaValidator.hpp"

#include <memory>

class DocumentModel
{
public:
  using ValidatorPtr = std::shared_ptr<JsonSchemaValidator>;
  using DocumentStorePtr = std::shared_ptr<DocumentStore>;

  DocumentModel(const nlohmann::json& document = nlohmann::json(json::value_t::string),
                const DocumentStorePtr& store = DocumentStorePtr(new JsonDocumentStore),
                const ValidatorPtr& schemaValidator = ValidatorPtr());
  ~DocumentModel();

  void addAt(const json::json_pointer& path, const json& value);
  void replaceAt(const json::json_pointer& path, const json& value);
  void deleteAt(const json::json_pointer& path);

  const json& documentJson() const;

private:
  DocumentStorePtr m_doc;
  ValidatorPtr m_validator;

  void editTemplate(
    const json::json_pointer& path,
    const json& value,
    std::function<void(json&, json::json_pointer&, const json&)> tryEditFn,
    std::function<void(DocumentStorePtr&, const json::json_pointer&, const json&)> editFn);
  bool validateDocument(const json& document);
  const json::json_pointer getNearestHavingClassAncestorPath(
    const json::json_pointer& editPath) const;
  void calculateRelativePath(const json::json_pointer& ancestorPath,
                             const json::json_pointer& currentPath,
                             json::json_pointer& relativePath);
  void save();
};

#endif