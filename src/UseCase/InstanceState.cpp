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

#include "UseCase/InstanceState.hpp"
#include <vector>
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Domain/Layout/Layout.hpp"
#include "Domain/Layout/LayoutNode.hpp"
#include "Domain/Model/Element.hpp"
#include "Utility/Log.hpp"

#undef DEBUG
#define DEBUG(msg, ...)

namespace VGG
{

InstanceState::InstanceState(
  std::shared_ptr<LayoutNode>           page,
  std::shared_ptr<Layout::ExpandSymbol> expander,
  std::shared_ptr<Layout::Layout>       layout)
  : m_page{ page }
  , m_expander{ expander }
  , m_layout{ layout }
{
  ASSERT(page);
  ASSERT(expander);
  ASSERT(layout);
}

InstanceState::Result InstanceState::setState(
  const std::string& instanceDescendantId,
  const std::string& listenerId,
  const std::string& stateMasterId)
{
  // todo: now find closest enclosing instance node, should find the right instance node
  auto instanceNode = findInstanceNode(instanceDescendantId, listenerId, stateMasterId);
  if (!instanceNode)
    return { false };

  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);

  auto ret = makeResultWithOldTree(instanceNode);
  pInstance->resetState();
  const auto& oldMasterId = pInstance->masterId();
  if (oldMasterId == stateMasterId)
  {
    DEBUG(
      "instance %s, set state, same master id %s, return",
      instanceNode->id().c_str(),
      oldMasterId.c_str());
    return { false };
  }

  DEBUG(
    "instance: %s, set state, old master id %s, new master id: %s; event target: %s, listener: %s",
    instanceNode->id().c_str(),
    pInstance->masterId().c_str(),
    stateMasterId.c_str(),
    instanceDescendantId.c_str(),
    listenerId.c_str());

  setState(instanceNode, stateMasterId);
  return ret;
}

InstanceState::Result InstanceState::presentState(
  const std::string& instanceDescendantId,
  const std::string& listenerId,
  const std::string& stateMasterId,
  StateTree*         stateTree)
{
  ASSERT(stateTree);
  auto instanceNode = findInstanceNode(instanceDescendantId, listenerId, stateMasterId);
  if (!instanceNode)
    return { false };

  // save current state tree to still handle events
  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);
  const auto& oldMasterId = pInstance->masterId();
  if (oldMasterId == stateMasterId)
  {
    DEBUG(
      "instance %s, present state, same master id %s, return",
      instanceNode->id().c_str(),
      oldMasterId.c_str());
    return { false };
  }
  pInstance->saveOverrideTreeIfNeeded(); // Before cleaning up the child nodes

  auto ret = makeResultWithOldTree(instanceNode);
  auto oldElementChildren = pInstance->presentState(stateMasterId); // Clean up the child nodes
  auto oldLayoutChildren = m_layout->removeNodeChildren(instanceNode.get());

  DEBUG(
    "instance %s, present state, old master id %s, new master id: %s; event target: %s, listener "
    "id: %s",
    instanceNode->id().c_str(),
    oldMasterId.c_str(),
    stateMasterId.c_str(),
    instanceDescendantId.c_str(),
    listenerId.c_str());

  auto treeElement = std::make_shared<Domain::StateTreeElement>(pInstance->shared_from_this());
  for (auto& child : oldElementChildren)
  {
    treeElement->addChild(child);
  }

  stateTree->setSrcNode(instanceNode);
  stateTree->setTreeElement(treeElement);
  for (auto& child : oldLayoutChildren)
  {
    stateTree->addChild(child);
  }

  setState(instanceNode, stateMasterId);
  return ret;
}

InstanceState::Result InstanceState::dismissState(
  const StateTree*   savedStateTree,
  const std::string& instanceDescendantId)
{
  if (!savedStateTree)
    return { false };

  auto instanceNode = savedStateTree->srcNode();
  if (!instanceNode)
    return { false };

  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);
  DEBUG(
    "instance %s, dismiss state, master id %s; event target: %s",
    instanceNode->id().c_str(),
    pInstance->masterId().c_str(),
    instanceDescendantId.c_str());

  auto ret = makeResultWithOldTree(instanceNode);
  auto oldMasterId = pInstance->dissmissState();
  DEBUG(
    "instance %s, dismiss state, back to master id %s",
    instanceNode->id().c_str(),
    oldMasterId.c_str());

  setState(instanceNode, oldMasterId);

  return ret;
}

void InstanceState::setState(
  std::shared_ptr<LayoutNode> instanceNode,
  const std::string&          stateMasterId)
{
  ASSERT(instanceNode);

  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);
  pInstance->saveOverrideTreeIfNeeded();

  // expand again
  m_expander->expandInstance(*pInstance, stateMasterId);
}

std::shared_ptr<LayoutNode> InstanceState::findInstanceNode(
  const std::string& instanceDescendantId,
  const std::string& listenerId,
  const std::string& newVariantId)
{
  auto node = m_page->findDescendantNodeById(instanceDescendantId);
  if (!node)
  {
    return nullptr;
  }

  std::shared_ptr<LayoutNode> instanceNode;
  while (node)
  {
    if (
      (node->elementNode()->type() == Domain::Element::EType::SYMBOL_INSTANCE) &&
      m_expander->isSameComponent(
        static_cast<Domain::SymbolInstanceElement*>(node->elementNode())->masterId(),
        newVariantId))
    {
      instanceNode = node->shared_from_this();
      break;
    }
    node = node->parent();
  }
  return instanceNode;
}

InstanceState::Result InstanceState::makeResultWithOldTree(
  const std::shared_ptr<LayoutNode>& instanceNode)
{
  ASSERT(instanceNode);
  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);

  Result ret;
  ret.success = true;

  // old
  auto ele = pInstance->cloneTree();
  ret.oldTree = std::make_shared<SnapshotTree>(ele);
  Layout::Layout::buildSubtree(ret.oldTree.get());

  // new
  pInstance->regenerateId(true);
  ret.newTree = instanceNode;

  return ret;
}

} // namespace VGG