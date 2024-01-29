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

// #define DUMP_TREE

using namespace VGG;
using namespace VGG::Layout::Internal::Rule;

Layout::Layout::Layout(JsonDocumentPtr designDoc, JsonDocumentPtr layoutDoc)
{
  RuleMapPtr rules;
  if (layoutDoc)
  {
    rules = collectRules(layoutDoc->content());
  }

  new (this) Layout(designDoc, rules);
}

Layout::Layout::Layout(JsonDocumentPtr designDoc, RuleMapPtr rules)
  : m_designDoc{ designDoc }
  , m_rules{ rules }
{
  ASSERT(m_designDoc);

  if (!m_rules)
  {
    m_rules = std::make_shared<RuleMap>();
  }

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

  // udpate page frame
  for (auto& page : root->children())
  {
    page->scaleTo(size, updateRule, true);
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

  m_layoutTree.reset(new LayoutNode{ nlohmann::json::json_pointer{ "/" } });
  json::json_pointer framesPath{ "/frames" };
  for (std::size_t i = 0; i < designJson[K_FRAMES].size(); ++i)
  {
    auto path = framesPath / i;
    auto page = createOneLayoutNode(designJson[path], path, m_layoutTree);
    m_pageSize.push_back(page->frame().size);
  }
}

std::shared_ptr<LayoutNode> Layout::Layout::createOneLayoutNode(
  const nlohmann::json&       j,
  const json::json_pointer&   currentPath,
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

  auto node = std::make_shared<LayoutNode>(currentPath);
  node->setViewModel(m_designDoc);
  if (parent)
  {
    parent->addChild(node);
  }

  buildSubtree(j, currentPath, node);

  return node;
}

void Layout::Layout::buildSubtree(
  const nlohmann::json&               j,
  const nlohmann::json::json_pointer& currentPath,
  std::shared_ptr<LayoutNode>         parent)
{
  if (j.contains(K_CHILD_OBJECTS))
  {
    auto path = currentPath / K_CHILD_OBJECTS;
    createLayoutNodes(j[K_CHILD_OBJECTS], path, parent);
  }

  if (j[K_CLASS] == K_PATH)
  {
    // ./shape/subshapes/i/subGeometry
    auto& subShapes = j[K_SHAPE][K_SUBSHAPES];
    auto  subShapesPath = currentPath / K_SHAPE / K_SUBSHAPES;
    auto  size = subShapes.size();
    for (std::size_t i = 0; i < size; ++i)
    {
      createOneLayoutNode(subShapes[i][K_SUBGEOMETRY], subShapesPath / i / K_SUBGEOMETRY, parent);
    }
  }
}

void Layout::Layout::createLayoutNodes(
  const nlohmann::json&       j,
  const json::json_pointer&   currentPath,
  std::shared_ptr<LayoutNode> parent)
{
  if (!j.is_array())
  {
    WARN("create layout nodes from json array, json is not array, return");
    return;
  }

  auto size = j.size();
  auto path = currentPath;
  for (std::size_t i = 0; i < size; ++i)
  {
    path /= i;
    createOneOrMoreLayoutNodes(j[i], path, parent);
    path.pop_back();
  }
}

void Layout::Layout::createOneOrMoreLayoutNodes(
  const nlohmann::json&       j,
  const json::json_pointer&   currentPath,
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

Layout::Layout::RuleMapPtr Layout::Layout::collectRules(const nlohmann::json& json)
{
  auto result = std::make_shared<RuleMap>();

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
      auto  rule = std::make_shared<Internal::Rule::Rule>();
      *rule = item;

      (*result)[id] = rule;
    }
  }

  return result;
}

void Layout::Layout::configureNodeAutoLayout(
  std::shared_ptr<LayoutNode> node,
  bool                        createAutoLayout)
{
  std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule;

  auto& designJson = m_designDoc->content();
  if (node->path() != "/")
  {
    auto& json = designJson[nlohmann::json::json_pointer{ node->path() }];
    if (json.contains(K_ID))
    {
      auto nodeId = json.at(K_ID).get<std::string>();
      if (m_rules->find(nodeId) != m_rules->end())
      {
        rule = (*m_rules)[nodeId];
      }
    }
  }

  auto autoLayout = node->autoLayout();
  if (createAutoLayout || !autoLayout)
  {
    autoLayout = node->createAutoLayout();
  }

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
  for (const auto& [id, rule] : *m_rules)
  {
    if (auto flexContainerRule = rule->getFlexContainerRule();
        flexContainerRule && flexContainerRule->zOrder)
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
    if (m_rules->find(nodeId) != m_rules->end())
    {
      auto rule = (*m_rules)[nodeId];
      if (auto flexContainerRule = rule->getFlexContainerRule();
          flexContainerRule && flexContainerRule->zOrder)
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

void Layout::Layout::resizeNodeThenLayout(
  const std::string& nodeId,
  Size               size,
  bool               preservingOrigin)
{
  if (auto node = m_layoutTree->findDescendantNodeById(nodeId))
  {
    resizeNodeThenLayout(node, size, preservingOrigin);
  }
  else
  {
    WARN("Layout::resizeNodeThenLayout: subtree not found, nodeId: %s", nodeId.c_str());
  }
}

void Layout::Layout::resizeNodeThenLayout(
  std::shared_ptr<LayoutNode> node,
  Size                        size,
  bool                        preservingOrigin)
{
  if (!node)
  {
    return;
  }

  DEBUG("Layout::resizeNodeThenLayout: resize subtree, %s", node->id().c_str());
  auto treeToLayout = node->scaleTo(size, true, preservingOrigin);
  if (treeToLayout)
  {
    treeToLayout->layoutIfNeeded();
  }
  else
  {
    DEBUG("Layout::resizeNodeThenLayout: no subtree to layou");
  }
}

void Layout::Layout::layoutNodes(
  const std::vector<std::string>& nodeIds,
  const std::string&              constainerNodeId)
{
  auto containerNode = m_layoutTree->findDescendantNodeById(constainerNodeId);
  if (!containerNode)
  {
    DEBUG("Layout::layoutNodes: container node not found, %s", constainerNodeId.c_str());
    return;
  }

  std::vector<std::shared_ptr<LayoutNode>> subtrees;
  for (const auto& nodeId : nodeIds)
  {
    if (auto node = containerNode->findDescendantNodeById(nodeId))
    {
      DEBUG("Layout::layoutNodes: set flex container need to layout, %s", nodeId.c_str());
      auto subtree = node->autoLayout()->setNeedsLayout();
      if (subtree)
      {
        subtrees.push_back(subtree);
      }
    }
  }

  // get closed common ancestor
  std::shared_ptr<LayoutNode> commonAncestor;
  for (auto& subtree : subtrees)
  {
    if (!commonAncestor)
    {
      commonAncestor = subtree;
    }
    else
    {
      commonAncestor = commonAncestor->closestCommonAncestor(subtree);
    }
  }

  if (commonAncestor)
  {
    commonAncestor->layoutIfNeeded();
  }
}

void Layout::Layout::rebuildSubtree(std::shared_ptr<LayoutNode> node)
{
  if (!node)
  {
    return;
  }

  DEBUG("Layout::rebuildSubtree: node id is %s, %s", node->id().c_str(), node->path().c_str());
  node->removeAllChildren();

  const auto& path = node->jsonPointer();
  const auto& json = m_designDoc->content()[path];

  buildSubtree(json, path, node);
  configureNodeAutoLayout(node, false);

#ifdef DUMP_TREE
  m_layoutTree->dump();
#endif
}

void Layout::Layout::rebuildSubtreeById(std::string nodeId)
{
  if (auto node = m_layoutTree->findDescendantNodeById(nodeId))
  {
    rebuildSubtree(node);
  }
  else
  {
    DEBUG("Layout::rebuildSubtreeById: node not found, nodeId: %s", nodeId.c_str());
#ifdef DUMP_TREE
    m_layoutTree->dump();
#endif
  }
}