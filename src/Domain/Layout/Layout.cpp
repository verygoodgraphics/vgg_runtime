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
#include "Layout.hpp"

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "RawJsonDocument.hpp"
#include "Rule.hpp"
#include "Utility/Log.hpp"

#include <algorithm>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;
using namespace VGG::Layout::Internal::Rule;

Layout::Layout::Layout(JsonDocumentPtr designDoc, JsonDocumentPtr layoutDoc, bool isRootTree)
{
  RuleMap rules;
  if (layoutDoc)
  {
    rules = collectRules(layoutDoc->content());
  }

  new (this) Layout(designDoc, rules, isRootTree);
}

Layout::Layout::Layout(JsonDocumentPtr designDoc, RuleMap rules, bool isRootTree)
  : m_designDoc{ designDoc }
  , m_rules{ rules }
  , m_isRootTree{ isRootTree }
{
  ASSERT(m_designDoc);

  // initial config
  buildLayoutTree();
  configureNodeAutoLayout(m_layoutTree);
}

void Layout::Layout::layout(Size size, bool updateRule)
{
  // todo, layout current page only

  // update root frame
  auto root = layoutTree();
  auto frame = root->frame();

  // Always update the root size because resizing the page does not update the root size
  frame.size = size;

  if (m_isRootTree)
  {
    // udpate page frame
    for (auto& page : root->children())
    {
      page->scaleTo(size, updateRule);
    }
  }
  else
  {
    root->setFrame(frame, updateRule);
    root->autoLayout()->setNeedsLayout();
  }

  // layout
  root->layoutIfNeeded();
}

void Layout::Layout::buildLayoutTree()
{
  auto& designJson = m_designDoc->content();
  if (!designJson.is_object())
  {
    WARN("invalid design file");
    return;
  }

  if (m_isRootTree)
  {
    m_layoutTree.reset(new LayoutNode{ "/" });
    json::json_pointer framesPath{ "/frames" };
    for (auto i = 0; i < designJson[K_FRAMES].size(); ++i)
    {
      auto path = framesPath / i;
      auto page = createOneLayoutNode(designJson[path], path, m_layoutTree);
      m_pageSize.push_back(page->frame().size);
    }
  }
  else
  {
    json::json_pointer path;
    m_layoutTree = createOneLayoutNode(designJson, path, nullptr);
  }
}

std::shared_ptr<LayoutNode> Layout::Layout::createOneLayoutNode(const nlohmann::json& j,
                                                                json::json_pointer currentPath,
                                                                std::shared_ptr<LayoutNode> parent)
{
  if (!j.is_object())
  {
    WARN("create one layout node from json object, json is not object, return");
    return nullptr;
  }

  if (!isLayoutNode(j))
  {
    return nullptr;
  }

  auto node = std::make_shared<LayoutNode>(currentPath.to_string());
  node->setViewModel(m_designDoc);
  if (parent)
  {
    parent->addChild(node);
  }

  for (auto& [key, val] : j.items())
  {
    auto path = currentPath;
    path /= key;

    createOneOrMoreLayoutNodes(val, path, node);
  }

  if (j[K_CLASS] == K_PATH)
  {
    // ./shape/subshapes/i/subGeometry
    auto& subShapes = j[K_SHAPE][K_SUBSHAPES];
    auto subShapesPath = currentPath / K_SHAPE / K_SUBSHAPES;
    auto size = subShapes.size();
    for (auto i = 0; i < size; ++i)
    {
      createOneLayoutNode(subShapes[i][K_SUBGEOMETRY], subShapesPath / i / K_SUBGEOMETRY, node);
    }
  }

  return node;
}

void Layout::Layout::createLayoutNodes(const nlohmann::json& j,
                                       json::json_pointer currentPath,
                                       std::shared_ptr<LayoutNode> parent)
{
  if (!j.is_array())
  {
    WARN("create layout nodes from json array, json is not array, return");
    return;
  }

  auto size = j.size();
  for (auto i = 0; i < size; ++i)
  {
    auto path = currentPath;
    path /= i;

    createOneOrMoreLayoutNodes(j[i], path, parent);
  }
}

void Layout::Layout::createOneOrMoreLayoutNodes(const nlohmann::json& j,
                                                json::json_pointer currentPath,
                                                std::shared_ptr<LayoutNode> parent)
{
  if (j.is_object())
  {
    createOneLayoutNode(j, currentPath, parent);
  }
  else if (j.is_array())
  {
    createLayoutNodes(j, currentPath, parent);
  }
}

Layout::Layout::RuleMap Layout::Layout::collectRules(const nlohmann::json& json)
{
  RuleMap result;

  if (!json.is_object())
  {
    return result;
  }

  auto& obj = json[K_OBJ];
  if (obj.is_array())
  {
    for (auto& item : obj)
    {
      if (!item.is_object() || !item.contains(K_ID))
      {
        continue;
      }

      auto& id = item[K_ID];
      auto rule = std::make_shared<Internal::Rule::Rule>();
      *rule = item;

      result[id] = rule;
    }
  }

  return result;
}

void Layout::Layout::configureNodeAutoLayout(std::shared_ptr<LayoutNode> node)
{
  std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule;

  auto& designJson = m_designDoc->content();
  if (node->path() != "/")
  {
    auto& json = designJson[nlohmann::json::json_pointer{ node->path() }];
    if (json.contains(K_ID))
    {
      auto nodeId = json.at(K_ID).get<std::string>();
      if (m_rules.find(nodeId) != m_rules.end())
      {
        rule = m_rules[nodeId];
      }
    }
  }

  auto autoLayout = node->createAutoLayout();
  if (rule)
  {
    autoLayout->rule = rule;
    node->configureAutoLayout();
  }

  for (auto& child : node->children())
  {
    configureNodeAutoLayout(child);
  }
}

bool Layout::Layout::hasFirstOnTopNode()
{
  for (const auto& [id, rule] : m_rules)
  {
    if (auto flexContainerRule = rule->getFlexContainerRule();
        flexContainerRule && flexContainerRule->z_order)
    {
      return true;
    }
  }

  return false;
}

JsonDocumentPtr Layout::Layout::displayDesignDoc()
{
  if (hasFirstOnTopNode())
  {
    auto designJson = m_designDoc->content();
    reverseChildren(designJson);

    JsonDocumentPtr result{ new RawJsonDocument() };
    result->setContent(designJson);
    return result;
  }

  return m_designDoc;
}

void Layout::Layout::reverseChildren(nlohmann::json& json)
{
  if (!json.is_object() && !json.is_array())
  {
    return;
  }

  if (json.contains(K_ID) && json.contains(K_CHILD_OBJECTS))
  {
    auto nodeId = json[K_ID].get<std::string>();
    if (m_rules.find(nodeId) != m_rules.end())
    {
      auto rule = m_rules[nodeId];
      if (auto flexContainerRule = rule->getFlexContainerRule();
          flexContainerRule && flexContainerRule->z_order)
      {
        auto& children = json[K_CHILD_OBJECTS];
        if (children.is_array())
        {
          std::reverse(children.begin(), children.end());
        }
      }
    }
  }

  for (auto& el : json.items())
  {
    reverseChildren(el.value());
  }
}

void Layout::Layout::resizeNodeThenLayout(const std::string& nodeId, Size size)
{
  if (auto node = m_layoutTree->findDescendantNodeById(nodeId))
  {
    DEBUG("Layout::resizeNodeThenLayout: resize subtree, %s", nodeId.c_str());
    node->scaleTo(size, true);

    m_layoutTree->layoutIfNeeded();
  }
}

void Layout::Layout::layoutNodes(const std::vector<std::string>& nodeIds)
{
  for (const auto& nodeId : nodeIds)
  {
    if (auto node = m_layoutTree->findDescendantNodeById(nodeId))
    {
      DEBUG("Layout::layoutNodes: set flex container need to layout, %s", nodeId.c_str());
      node->autoLayout()->setNeedsLayout();
    }
  }

  m_layoutTree->layoutIfNeeded();
}