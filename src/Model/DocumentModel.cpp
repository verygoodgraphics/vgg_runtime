#include "DocumentModel.hpp"

DocumentModel::DocumentModel(const nlohmann::json& schema, const nlohmann::json& document)
{
  validator_.setRootSchema(schema);
  if (validator_.validate(document))
  {
    from_json(document, doc_);
  }
  else
  {
    throw std::runtime_error("Invalid document");
  }
}

DocumentModel::~DocumentModel()
{
}

void DocumentModel::addAt(const json::json_pointer& path, const json& value)
{
  validate();
  doc_.json_add(path, value);
}

void DocumentModel::replaceAt(const json::json_pointer& path, const json& value)
{
  validate();
  doc_.json_replace(path, value);
}

void DocumentModel::deleteAt(const json::json_pointer& path)
{
  validate();
  doc_.json_delete(path);
}

json DocumentModel::documentJson()
{
  return doc_;
}

void DocumentModel::validate()
{
}

void DocumentModel::save()
{
}