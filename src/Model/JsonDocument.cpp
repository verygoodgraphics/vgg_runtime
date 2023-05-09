#include "JsonDocument.hpp"

const json& JsonDocument::content() const
{
  return m_jsonDoc->content();
}
void JsonDocument::setContent(const json& document)
{
  m_jsonDoc->setContent(document);
}

void JsonDocument::addAt(const std::string& path, const std::string& value)
{
  addAt(json::json_pointer(path), json::parse(value));
}
void JsonDocument::replaceAt(const std::string& path, const std::string& value)
{
  replaceAt(json::json_pointer(path), json::parse(value));
}
void JsonDocument::deleteAt(const std::string& path)
{
  deleteAt(json::json_pointer(path));
}

void JsonDocument::addAt(const json::json_pointer& path, const json& value)
{
  m_jsonDoc->addAt(path, value);
}
void JsonDocument::replaceAt(const json::json_pointer& path, const json& value)
{
  m_jsonDoc->replaceAt(path, value);
}
void JsonDocument::deleteAt(const json::json_pointer& path)
{
  m_jsonDoc->deleteAt(path);
}

void JsonDocument::undo()
{
  m_jsonDoc->undo();
}
void JsonDocument::redo()
{
  m_jsonDoc->redo();
}
void JsonDocument::save()
{
  m_jsonDoc->save();
}

void JsonDocument::erase(json& target, const json::json_pointer& path)
{
  auto& j = target.at(path.parent_pointer());
  if (j.is_object())
  {
    j.erase(path.back());
  }
  else // array
  {
    auto index = std::stoul(path.back());
    j.erase(index);
  }
}