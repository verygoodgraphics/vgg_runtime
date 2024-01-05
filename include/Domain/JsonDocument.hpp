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
#pragma once

#include <nlohmann/json.hpp>

#include <memory>

class JsonDocument;
using JsonDocumentPtr = std::shared_ptr<JsonDocument>;
using json = nlohmann::json;

class JsonDocument
{
public:
  JsonDocument(JsonDocumentPtr jsonDoc = JsonDocumentPtr())
    : m_jsonDoc(jsonDoc)
  {
  }
  virtual ~JsonDocument() = default;

  virtual const json& content() const;
  virtual void        setContent(const json& document);

  virtual void addAt(const std::string& path, const std::string& value);
  virtual void replaceAt(const std::string& path, const std::string& value);
  virtual void deleteAt(const std::string& path);

  virtual void addAt(const json::json_pointer& path, const json& value);
  virtual void replaceAt(const json::json_pointer& path, const json& value);
  virtual void deleteAt(const json::json_pointer& path);

  virtual void undo();
  virtual void redo();
  virtual void save();

public:
  static void erase(json& target, const json::json_pointer& path);

public:
  std::string getElement(const std::string& id);
  void        updateElement(const std::string& id, const std::string& contentJsonString);

private:
  void updateElementInTree(
    const nlohmann::json&     tree,
    const json::json_pointer& treePath,
    const std::string&        id,
    const std::string&        contentJsonString);

protected:
  JsonDocumentPtr m_jsonDoc;
};
