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

#include "AutoLayout.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "RawJsonDocument.hpp"
#include "Rule.hpp"
#include "Utility/Log.hpp"

#include "Domain/Model/DesignModel.hpp"
#include "Domain/Model/Element.hpp"

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
  m_layoutTree.reset(new LayoutNode{ m_designDocument });
  for (auto& child : m_designDocument->children())
  {
    auto page = createOneLayoutNode(child, m_layoutTree);
    m_pageSize.push_back(page->frame().size);
  }
}

void Layout::Layout::buildSubtree(std::shared_ptr<LayoutNode> parent)
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
    createOneLayoutNode(child, parent);
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
    configureNodeAutoLayout(child);
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

  buildSubtree(node);
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

std::shared_ptr<LayoutNode> Layout::Layout::createOneLayoutNode(
  std::shared_ptr<Domain::Element> element,
  std::shared_ptr<LayoutNode>      parent)
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
    createOneLayoutNode(child, node);
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