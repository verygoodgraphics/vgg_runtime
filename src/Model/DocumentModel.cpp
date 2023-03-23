#include "DocumentModel.hpp"

#include "Utils/Utils.hpp"

constexpr auto const_class_name = "class";

DocumentModel::DocumentModel(std::shared_ptr<JsonSchemaValidator> schemaValidator,
                             const nlohmann::json& document)
  : m_validator(schemaValidator)
{
  from_json(document, m_doc);
}

DocumentModel::~DocumentModel()
{
}

void DocumentModel::addAt(const json::json_pointer& path, const json& value)
{
  editTemplate(
    path,
    value,
    [](json& tmp_document, json::json_pointer& relative_path, const json& cb_value)
    { tmp_document[relative_path] = cb_value; },
    [](Automerge& cb_doc, const json::json_pointer& cb_path, const json& cb_value)
    { cb_doc.json_add(cb_path, cb_value); });
}

void DocumentModel::replaceAt(const json::json_pointer& path, const json& value)
{
  editTemplate(
    path,
    value,
    [](json& tmp_document, json::json_pointer& relative_path, const json& cb_value)
    { tmp_document[relative_path] = cb_value; },
    [](Automerge& cb_doc, const json::json_pointer& cb_path, const json& cb_value)
    { cb_doc.json_replace(cb_path, cb_value); });
}

void DocumentModel::deleteAt(const json::json_pointer& path)
{
  auto stub_value = documentJson();
  editTemplate(
    path,
    stub_value,
    [](json& tmp_document, json::json_pointer& relative_path, const json& cb_value)
    { tmp_document.at(relative_path.parent_pointer()).erase(relative_path.back()); },
    [](Automerge& cb_doc, const json::json_pointer& cb_path, const json& cb_value)
    { cb_doc.json_delete(cb_path); });
}

const json& DocumentModel::documentJson() const
{
  return m_doc.json_const_ref();
}

void DocumentModel::editTemplate(
  const json::json_pointer& path,
  const json& value,
  std::function<void(json&, json::json_pointer&, const json&)> tryEditFn,
  std::function<void(Automerge&, const json::json_pointer&, const json&)> editFn)
{
  auto ancestor_path = getNearestHavingClassAncestorPath(path);
  auto tmp_document = documentJson()[ancestor_path];

  json::json_pointer relative_path;
  calculateRelativePath(ancestor_path, path, relative_path);

  tryEditFn(tmp_document, relative_path, value);
  if (validateDocument(tmp_document))
  {
    editFn(m_doc, path, value);
  }
  else
  {
    throw std::logic_error("Invalid value");
  }
}

bool DocumentModel::validateDocument(const json& document)
{
  if (document.is_object() && document.contains(const_class_name) &&
      document[const_class_name].is_string())
  {
    auto class_name = document[const_class_name].get<std::string>();
    return m_validator->validate(class_name, document);
  }
  else
  {
    WARN("#DocumentModel::validate: fallback, validate whole document");
    return m_validator->validate(document);
  }
}

const json::json_pointer DocumentModel::getNearestHavingClassAncestorPath(
  const json::json_pointer& editPath) const
{
  const auto& document = documentJson();
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
    WARN("#DocumentModel::getNearestAncestorHavingClass: error: %d, %s", e.id, e.what());
  }

  WARN("#DocumentModel::getNearestAncestorHavingClass: return root path");
  return ""_json_pointer;
}

void DocumentModel::calculateRelativePath(const json::json_pointer& ancestorPath,
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

void DocumentModel::save()
{
  // todo: save to vgg backend
}