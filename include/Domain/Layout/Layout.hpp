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

#include "Domain/Daruma.hpp"
#include "Domain/Layout/Node.hpp"
#include "Utility/Log.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace VGG
{
namespace Domain
{
class DesignDocument;
class Element;
} // namespace Domain
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
public:
  using RuleMap = std::unordered_map<std::string, std::shared_ptr<Internal::Rule::Rule>>;
  using RuleMapPtr = std::shared_ptr<RuleMap>;

private:
  std::shared_ptr<Domain::DesignDocument> m_designDocument;
  std::shared_ptr<LayoutNode>             m_layoutTree;
  RuleMapPtr                              m_rules;
  std::vector<Size>                       m_originalPageSize;

public:
  Layout(JsonDocumentPtr designDoc, JsonDocumentPtr layoutDoc);
  Layout(JsonDocumentPtr designDoc, RuleMapPtr rules);
  Layout(std::shared_ptr<Domain::DesignDocument> designDocument, RuleMapPtr rules);

  void layout(Size size, int pageIndex, bool updateRule = false);
  void resizeNodeThenLayout(const std::string& nodeId, Size size, bool preservingOrigin);
  void resizeNodeThenLayout(std::shared_ptr<LayoutNode> node, Size size, bool preservingOrigin);
  void layoutNodes(const std::vector<std::string>& nodeIds, const std::string& constainerNodeId);

  void rebuildSubtree(std::shared_ptr<LayoutNode> node);
  void rebuildSubtreeById(std::string nodeId);

  std::shared_ptr<LayoutNode> layoutTree() const
  {
    return m_layoutTree;
  }

  auto originalPageSize(int index)
  {
    return m_originalPageSize[index];
  }

  std::shared_ptr<Domain::DesignDocument> designDocTree();
  JsonDocumentPtr                         displayDesignDoc();

public:
  static RuleMapPtr collectRules(const nlohmann::json& json);

private:
  void                        buildLayoutTree();
  std::shared_ptr<LayoutNode> createOneLayoutNode(
    std::shared_ptr<Domain::Element> element,
    std::shared_ptr<LayoutNode>      parent);

  void buildSubtree(std::shared_ptr<LayoutNode> parent);

  void configureNodeAutoLayout(std::shared_ptr<LayoutNode> node, bool createAutoLayout = true);

  void updateFirstOnTop(std::shared_ptr<Domain::Element> element);
};

} // namespace Layout
} // namespace VGG
