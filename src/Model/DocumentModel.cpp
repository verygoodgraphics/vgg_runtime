#include "DocumentModel.hpp"

#include "Utils/Utils.hpp"

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
  if (validate(tmp_document, path.parent_pointer()))
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
  if (validate(tmp_document, path.parent_pointer()))
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
  if (validate(tmp_document, path.parent_pointer()))
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
  try
  {
    auto path_to_validate = path;
    do
    {
      auto json_to_validate = document[path_to_validate];
      if (json_to_validate.is_object())
      {
        try
        {
          auto class_json = json_to_validate["class"];
          if (class_json.is_string())
          {
            auto class_name = class_json.get<std::string>();
            return validator_.validate(class_name, json_to_validate);
          }
        }
        catch (json::type_error& e)
        {
          WARN("#DocumentModel::validate: no class field, error: %d, %s", e.id, e.what());
        }
      }
      path_to_validate = path_to_validate.parent_pointer();
    } while (true);
  }
  catch (json::type_error& e)
  {
    WARN("#DocumentModel::validate: error: %d, %s", e.id, e.what());
  }

  WARN("#DocumentModel::validate: fallback, validate whole document");
  return validator_.validate(document);
}

void DocumentModel::save()
{
}