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
  auto tmp_document = documentJson();
  tmp_document[path] = value;
  if (validate(tmp_document, path))
  {
    doc_.json_add(path, value);
  }
  else
  {
    throw std::logic_error("Invalid value");
  }
}

void DocumentModel::replaceAt(const json::json_pointer& path, const json& value)
{
  auto tmp_document = documentJson();
  tmp_document[path] = value;
  if (validate(tmp_document, path))
  {
    doc_.json_replace(path, value);
  }
  else
  {
    throw std::logic_error("Invalid value");
  }
}

void DocumentModel::deleteAt(const json::json_pointer& path)
{
  auto tmp_document = documentJson();
  tmp_document.at(path.parent_pointer()).erase(path.back());
  if (validate(tmp_document, path))
  {
    doc_.json_delete(path);
  }
  else
  {
    throw std::logic_error("Invalid value");
  }
}

json DocumentModel::documentJson()
{
  return doc_;
}

bool DocumentModel::validate(const json& document, const json::json_pointer& path)
{
  // todo: validate by class name
  return validator_.validate(document);
}

void DocumentModel::save()
{
}