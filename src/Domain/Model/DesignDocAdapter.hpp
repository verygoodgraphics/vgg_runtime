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

namespace VGG
{
namespace Domain
{
class DesignDocument;
}
} // namespace VGG

namespace VGG::Model
{

class DesignDocAdapter : public JsonDocument
{
  std::shared_ptr<VGG::Domain::DesignDocument> m_designDocTree;

public:
  DesignDocAdapter(std::shared_ptr<VGG::Domain::DesignDocument> designDocTree);

  json        content() const override;
  std::string getElement(const std::string& id) override;
  void        updateElement(const std::string& id, const std::string& contentJsonString) override;

  void setContent(const nlohmann::json& content) override
  {
  }

  void addAt(const json::json_pointer& path, const json& value) override
  {
  }

  void replaceAt(const json::json_pointer& path, const json& value) override
  {
  }

  void deleteAt(const json::json_pointer& path) override
  {
  }
};

} // namespace VGG::Model