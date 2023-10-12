/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
