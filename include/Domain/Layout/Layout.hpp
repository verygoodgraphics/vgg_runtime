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

#include "Domain/Daruma.hpp"
#include "ExpandSymbol.hpp"
#include "Utility/Log.hpp"
#include "Node.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace VGG
{
namespace Layout
{
namespace Internal
{
namespace Rule
{
struct Rule;
}
} // namespace Internal

class Layout
{
  using RuleMap = std::unordered_map<std::string, std::shared_ptr<Internal::Rule::Rule>>;

  JsonDocumentPtr m_designDoc;
  JsonDocumentPtr m_layoutDoc;
  std::shared_ptr<LayoutNode> m_layoutTree;
  RuleMap m_rules;
  bool m_isRootTree{ true }; // root document or fragment
  std::vector<Size> m_pageSize;

public:
  Layout(JsonDocumentPtr designDoc, JsonDocumentPtr layoutDoc, bool isRootTree = true);

  void layout(Size size, bool updateRule = false);
  void resizeNodeThenLayout(const std::string& nodeId, Size size);

  std::shared_ptr<LayoutNode> layoutTree() const
  {
    return m_layoutTree;
  }

  auto pageSize(int index)
  {
    return m_pageSize[index];
  }

  JsonDocumentPtr displayDesignDoc();

private:
  void buildLayoutTree();
  std::shared_ptr<LayoutNode> createOneLayoutNode(const nlohmann::json& j,
                                                  nlohmann::json::json_pointer currentPath,
                                                  std::shared_ptr<LayoutNode> parent);
  void createLayoutNodes(const nlohmann::json& j,
                         nlohmann::json::json_pointer currentPath,
                         std::shared_ptr<LayoutNode> parent);

  void createOneOrMoreLayoutNodes(const nlohmann::json& j,
                                  nlohmann::json::json_pointer currentPath,
                                  std::shared_ptr<LayoutNode> parent);
  void collectRules(const nlohmann::json& json);
  void configureNodeAutoLayout(std::shared_ptr<LayoutNode> node);

  bool hasFirstOnTopNode();
  void reverseChildren(nlohmann::json& nodeJson);
};

} // namespace Layout
} // namespace VGG
