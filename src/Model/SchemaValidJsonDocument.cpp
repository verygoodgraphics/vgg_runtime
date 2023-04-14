#include "SchemaValidJsonDocument.hpp"

#include "Utils/Utils.hpp"

using json = nlohmann::json;

constexpr auto const_class_name = "class";

SchemaValidJsonDocument::SchemaValidJsonDocument(
  const JsonDocumentPtr& jsonDoc = JsonDocumentPtr(),
  const ValidatorPtr& schemaValidator = ValidatorPtr())
  : JsonDocument(jsonDoc)
  , m_validator(schemaValidator)
{
}

void SchemaValidJsonDocument::addAt(const json::json_pointer& path, const json& value)
{
  editTemplate(
    path,
    value,
    [](json& tmp_document, json::json_pointer& relative_path, const json& cb_value)
    { tmp_document[relative_path] = cb_value; },
    [](JsonDocumentPtr& cb_doc, const json::json_pointer& cb_path, const json& cb_value)
    { cb_doc->addAt(cb_path, cb_value); });
}

void SchemaValidJsonDocument::replaceAt(const json::json_pointer& path, const json& value)
{
  editTemplate(
    path,
    value,
    [](json& tmp_document, json::json_pointer& relative_path, const json& cb_value)
    { tmp_document[relative_path] = cb_value; },
    [](JsonDocumentPtr& cb_doc, const json::json_pointer& cb_path, const json& cb_value)
    { cb_doc->replaceAt(cb_path, cb_value); });
}

void SchemaValidJsonDocument::deleteAt(const json::json_pointer& path)
{
  auto stub_value = content();
  editTemplate(
    path,
    stub_value,
    [](json& tmp_document, json::json_pointer& relative_path, const json& cb_value)
    { tmp_document.at(relative_path.parent_pointer()).erase(relative_path.back()); },
    [](JsonDocumentPtr& cb_doc, const json::json_pointer& cb_path, const json& cb_value)
    { cb_doc->deleteAt(cb_path); });
}

const json& SchemaValidJsonDocument::content() const
{
  return m_jsonDoc->content();
}

void SchemaValidJsonDocument::setContent(const json& content)
{
  return m_jsonDoc->setContent(content);
}

void SchemaValidJsonDocument::editTemplate(
  const json::json_pointer& path,
  const json& value,
  std::function<void(json&, json::json_pointer&, const json&)> tryEditFn,
  std::function<void(JsonDocumentPtr&, const json::json_pointer&, const json&)> editFn)
{
  auto ancestor_path = getNearestHavingClassAncestorPath(path);
  auto tmp_document = content()[ancestor_path];

  json::json_pointer relative_path;
  calculateRelativePath(ancestor_path, path, relative_path);

  tryEditFn(tmp_document, relative_path, value);
  if (validateDocument(tmp_document))
  {
    editFn(m_jsonDoc, path, value);
  }
  else
  {
    throw std::logic_error("Invalid value");
  }
}

bool SchemaValidJsonDocument::validateDocument(const json& document)
{
  if (!m_validator)
  {
    return true;
  }

  if (document.is_object() && document.contains(const_class_name) &&
      document[const_class_name].is_string())
  {
    auto class_name = document[const_class_name].get<std::string>();
    return m_validator->validate(class_name, document);
  }
  else
  {
    WARN("#SchemaValidJsonDocument::validate: fallback, validate whole document");
    return m_validator->validate(document);
  }
}

const json::json_pointer SchemaValidJsonDocument::getNearestHavingClassAncestorPath(
  const json::json_pointer& editPath) const
{
  const auto& document = content();
  try
  {
    auto ancestor_path = editPath.parent_pointer();
    do
    {
      auto& ancestor_json = document[ancestor_path];
      if (ancestor_json.is_object() && ancestor_json.contains(const_class_name) &&
          ancestor_json[const_class_name].is_string())
      {
        return ancestor_path;
      }
      if (ancestor_path.empty())
      {
        break;
      }
      ancestor_path = ancestor_path.parent_pointer();
    } while (true);
  }
  catch (json::type_error& e)
  {
    WARN("#SchemaValidJsonDocument::getNearestAncestorHavingClass: error: %d, %s", e.id, e.what());
  }

  WARN("#SchemaValidJsonDocument::getNearestAncestorHavingClass: return root path");
  return ""_json_pointer;
}

void SchemaValidJsonDocument::calculateRelativePath(const json::json_pointer& ancestorPath,
                                                    const json::json_pointer& currentPath,
                                                    json::json_pointer& relativePath)
{
  if (currentPath == ancestorPath)
  {
    return;
  }
  auto& current = currentPath.back();
  calculateRelativePath(ancestorPath, currentPath.parent_pointer(), relativePath);
  relativePath.push_back(current);
}

void SchemaValidJsonDocument::save()
{
  m_jsonDoc->save();
}