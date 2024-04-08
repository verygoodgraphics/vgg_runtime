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

#include "JsonDocument.hpp"

#include "Automerge.h"

#include <memory>

class UndoRedoJsonDocument : public JsonDocument
{
public:
  virtual void setContent(const nlohmann::json& content) override
  {
    from_json(content, m_doc);
  }
  json content() const override
  {
    return m_doc.json_const_ref();
  }

  virtual void addAt(const json::json_pointer& path, const json& value) override
  {
    m_doc.json_add(path, value);
  }

  virtual void replaceAt(const json::json_pointer& path, const json& value) override
  {
    m_doc.json_replace(path, value);
  }

  virtual void deleteAt(const json::json_pointer& path) override
  {
    m_doc.json_delete(path);
  }

private:
  Automerge m_doc;
};
