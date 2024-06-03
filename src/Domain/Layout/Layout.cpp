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
#include "Layout.hpp"
#include <cstddef>
#include <new>
#include "AutoLayout.hpp"
#include "Domain/JsonDocument.hpp"
#include "Domain/Model/Element.hpp"
#include "JsonKeys.hpp"
#include "LayoutNode.hpp"
#include "RawJsonDocument.hpp"
#include "Rule.hpp"
#include "Utility/Log.hpp"
#include <nlohmann/json.hpp>

#undef DEBUG
#define DEBUG(msg, ...)

// #define DUMP_TREE

namespace VGG
{
namespace
{
using namespace VGG::Layout::Internal::Rule;
}

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
{
  ASSERT(designDoc);
  auto designDocument = std::make_shared<Domain::DesignDocument>(designDoc->content());
  designDocument->buildSubtree();
  new (this) Layout(designDocument, rules);
}

Layout::Layout::Layout(std::shared_ptr<Domain::DesignDocument> designDocument, RuleMapPtr rules)
  : m_designDocument{ designDocument }
  , m_rules{ rules }
{
  ASSERT(m_designDocument);

  if (!m_rules)
  {
    m_rules = std::make_shared<RuleMap>();
  }

  // initial config
  buildLayoutTree();
  configureNodeAutoLayout(m_layoutTree.get());
}

void Layout::Layout::layout(Size size, int pageIndex, bool updateRule, LayoutContext* context)
{
  auto root = layoutTree();
  if (pageIndex < 0 || static_cast<std::size_t>(pageIndex) >= root->children().size())
  {
    return;
  }

  // udpate page frame
  root->children()[pageIndex]->scaleTo(size, updateRule, true);

  // layout
  root->layoutIfNeeded(context);
}

void Layout::Layout::buildLayoutTree()
{
  m_layoutTree.reset(new LayoutNode{ m_designDocument });
  for (auto& child : m_designDocument->children())
  {
    auto page = makeTree(child, m_layoutTree.get());
    m_originalPageSize.push_back(page->frame().size);
  }
}

void Layout::Layout::buildSubtree(LayoutNode* parent)
{
  if (!parent)
  {
    return;
  }
  auto element = parent->elementNode();
  if (!element)
  {
    return;
  }

  for (auto& child : element->children())
  {
    makeTree(child, parent);
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

void Layout::Layout::configureNodeAutoLayout(LayoutNode* node, bool createAutoLayout)
{
  std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule;

  const auto& nodeId = node->id();
  if (m_rules->find(nodeId) != m_rules->end())
  {
    rule = (*m_rules)[nodeId];
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
    configureNodeAutoLayout(child.get());
  }
}

JsonDocumentPtr Layout::Layout::displayDesignDoc()
{
  auto designModel = designDocTree()->treeModel(true);

  JsonDocumentPtr result{ new RawJsonDocument() };
  result->setContent(designModel);
  return result;
}

std::shared_ptr<Domain::DesignDocument> Layout::Layout::designDocTree()
{
  updateFirstOnTop(m_designDocument);
  return m_designDocument;
}

void Layout::Layout::resizeNodeThenLayout(
  const std::string& nodeId,
  Size               size,
  bool               preservingOrigin)
{
  if (auto node = findNodeById(nodeId))
  {
    resizeNodeThenLayout(node, size, preservingOrigin);
  }
  else
  {
    WARN("Layout::resizeNodeThenLayout: subtree not found, nodeId: %s", nodeId.c_str());
  }
}

void Layout::Layout::resizeNodeThenLayout(LayoutNode* node, Size size, bool preservingOrigin)
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
  auto containerNode = findNodeById(constainerNodeId);
  if (!containerNode)
  {
    DEBUG("Layout::layoutNodes: container node not found, %s", constainerNodeId.c_str());
    return;
  }

  std::vector<std::shared_ptr<LayoutNode>> subtrees;
  for (const auto& nodeId : nodeIds)
  {
    if (auto node = findNodeInTreeById(containerNode, nodeId))
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
  LayoutNode* commonAncestor{ nullptr };
  for (auto& subtree : subtrees)
  {
    if (!commonAncestor)
    {
      commonAncestor = subtree.get();
    }
    else
    {
      commonAncestor = commonAncestor->closestCommonAncestor(subtree.get());
    }
  }

  if (commonAncestor)
  {
    commonAncestor->layoutIfNeeded();
  }
}

void Layout::Layout::rebuildSubtree(LayoutNode* node)
{
  if (!node)
  {
    return;
  }

  DEBUG("Layout::rebuildSubtree: node id is %s", node->id().c_str());
  removeNodeChildren(node);

  buildSubtree(node);
  configureNodeAutoLayout(node, false);

#ifdef DUMP_TREE
  m_layoutTree->dump();
#endif
}

void Layout::Layout::rebuildSubtreeById(std::string nodeId)
{
  if (auto node = findNodeById(nodeId))
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

std::shared_ptr<LayoutNode> Layout::Layout::makeTree(
  std::shared_ptr<Domain::Element> element,
  LayoutNode*                      parent)
{
  if (!element || !element->isLayoutNode())
  {
    return nullptr;
  }

  auto node = std::make_shared<LayoutNode>(element);
  if (parent)
  {
    parent->addChild(node);
  }

  for (auto& child : element->children())
  {
    makeTree(child, node.get());
  }

  return node;
}

void Layout::Layout::updateFirstOnTop(std::shared_ptr<Domain::Element> element)
{
  if (!element)
  {
    return;
  }

  element->setFirstOnTop(false);
  auto id = element->id();
  if (!id.empty())
  {
    if (auto it = m_rules->find(id); it != m_rules->end())
    {
      auto rule = it->second;
      if (auto flexContainerRule = rule->getFlexContainerRule();
          flexContainerRule && flexContainerRule->zOrder)
      {
        element->setFirstOnTop(true);
      }
    }
  }

  for (auto& child : element->children())
  {
    updateFirstOnTop(child);
  }
}

LayoutNode* Layout::Layout::findNodeById(const std::string& id)
{
  return findNodeInTreeById(m_layoutTree.get(), id);
}

LayoutNode* Layout::Layout::findNodeInTreeById(LayoutNode* tree, const std::string& id)
{
  if (!tree)
    return nullptr;

  if (m_nodeCacheMap.contains(id)) // cache hit
    return m_nodeCacheMap[id];

  auto p = tree->findDescendantNodeById(id);
  if (p)
    m_nodeCacheMap[id] = p; // cache result

  return p;
}

void Layout::Layout::invalidateNodeCache(LayoutNode* tree)
{
  if (const auto& id = tree->id(); !id.empty())
    m_nodeCacheMap.erase(id);

  for (auto& child : tree->children())
    invalidateNodeCache(child.get());

  // m_nodeCacheMap.clear();
}

std::vector<std::shared_ptr<LayoutNode>> Layout::Layout::removeNodeChildren(LayoutNode* node)
{
  if (!node)
  {
    return {};
  }

  for (auto& child : node->children())
    invalidateNodeCache(child.get());

  return node->removeAllChildren();
}

void Layout::Layout::cacheOneNode(LayoutNode* node)
{
  if (!node)
    return;

  if (const auto& id = node->id(); !id.empty())
    m_nodeCacheMap[id] = node;
}

void Layout::Layout::cacheTreeNodes(LayoutNode* tree)
{
  if (!tree)
    return;

  if (const auto& id = tree->id(); !id.empty())
    m_nodeCacheMap[id] = tree;

  for (auto& child : tree->children())
    cacheTreeNodes(child.get());
}

} // namespace VGG