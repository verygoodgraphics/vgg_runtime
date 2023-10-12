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

#include "JsonDocument.hpp"

#include <memory>

class ReferenceJsonDocument : public JsonDocument
{
public:
  ReferenceJsonDocument(nlohmann::json& json)
    : m_doc{ json }
  {
  }

  virtual void setContent(const nlohmann::json& content) override
  {
  }

  virtual const json& content() const override
  {
    return m_doc;
  }

  virtual void addAt(const json::json_pointer& path, const json& value) override
  {
    m_doc[path] = value;
  }

  virtual void replaceAt(const json::json_pointer& path, const json& value) override
  {
    m_doc[path] = value;
  }

  virtual void deleteAt(const json::json_pointer& path) override
  {
    erase(m_doc, path);
  }

private:
  json& m_doc;
};
