/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

class JsonDocument;
using JsonDocumentPtr = std::shared_ptr<JsonDocument>;

class JsonDocument
{
public:
  using json = nlohmann::json;

  JsonDocument(JsonDocumentPtr jsonDoc = JsonDocumentPtr())
    : m_jsonDoc(jsonDoc)
  {
  }
  virtual ~JsonDocument() = default;

  virtual json content() const;
  virtual void setContent(const json& document);

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
  virtual std::string getElement(const std::string& id);
  virtual void        updateElement(const std::string& id, const std::string& contentJsonString);

protected:
  JsonDocumentPtr m_jsonDoc;
};
