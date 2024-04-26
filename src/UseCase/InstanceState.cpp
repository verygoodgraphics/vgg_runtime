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

#include "Domain/Layout/ExpandSymbol.hpp"
#include "Domain/Layout/Node.hpp"
#include "Domain/Model/Element.hpp"
#include "Utility/Log.hpp"

using namespace VGG;

#undef DEBUG
#define DEBUG(msg, ...)

InstanceState::InstanceState(
  std::shared_ptr<LayoutNode>           page,
  std::shared_ptr<Layout::ExpandSymbol> expander)
  : m_page{ page }
  , m_expander{ expander }
{
  ASSERT(page);
  ASSERT(expander);
}

bool InstanceState::setState(
  const std::string& instanceDescendantId,
  const std::string& listenerId,
  const std::string& stateMasterId)
{
  // todo, check nested instance
  auto instanceNode = findInstanceNode(instanceDescendantId, listenerId);
  if (!instanceNode)
  {
    return false;
  }

  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);
  pInstance->resetState();

  DEBUG(
    "instance: %s, set state, old master id %s, new master id: %s; event target: %s, listener: %s",
    instanceNode->id().c_str(),
    pInstance->masterId().c_str(),
    stateMasterId.c_str(),
    instanceDescendantId.c_str(),
    listenerId.c_str());

  return setState(instanceNode, stateMasterId);
}

bool InstanceState::presentState(
  const std::string& instanceDescendantId,
  const std::string& listenerId,
  const std::string& stateMasterId,
  StateTree*         stateTree)
{
  ASSERT(stateTree);
  auto instanceNode = findInstanceNode(instanceDescendantId, listenerId);
  if (!instanceNode)
  {
    return false;
  }

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
    return false;
  }

  auto oldElementChildren = pInstance->presentState(stateMasterId);
  auto oldLayoutChildren = instanceNode->removeAllChildren();

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

  return setState(instanceNode, stateMasterId);
}

bool InstanceState::dismissState(
  const StateTree*   savedStateTree,
  const std::string& instanceDescendantId)
{
  if (!savedStateTree)
  {
    return false;
  }

  auto instanceNode = savedStateTree->srcNode();
  if (!instanceNode)
  {
    return false;
  }

  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);
  DEBUG(
    "instance %s, dismiss state, master id %s; event target: %s",
    instanceNode->id().c_str(),
    pInstance->masterId().c_str(),
    instanceDescendantId.c_str());

  auto oldMasterId = pInstance->dissmissState();
  DEBUG(
    "instance %s, dismiss state, back to master id %s",
    instanceNode->id().c_str(),
    oldMasterId.c_str());

  return setState(instanceNode, oldMasterId);
}

bool InstanceState::setState(
  std::shared_ptr<LayoutNode> instanceNode,
  const std::string&          stateMasterId)
{
  ASSERT(instanceNode);

  auto pInstance = static_cast<Domain::SymbolInstanceElement*>(instanceNode->elementNode());
  ASSERT(pInstance);

  // expand again
  m_expander->expandInstance(
    std::shared_ptr<Domain::SymbolInstanceElement>{ pInstance },
    stateMasterId);

  // update view model
  // render

  return true;
}

std::shared_ptr<LayoutNode> InstanceState::findInstanceNode(
  const std::string& instanceDescendantId,
  const std::string& listenerId)
{
  auto node = m_page->findDescendantNodeById(instanceDescendantId);
  if (!node)
  {
    return nullptr;
  }

  std::shared_ptr<LayoutNode> instanceNode;
  while (node)
  {
    if (const auto& id = node->id();
        node->elementNode()->type() == Domain::Element::EType::SYMBOL_INSTANCE &&
        ((id == listenerId) || !id.ends_with(listenerId)))
    {
      instanceNode = node;
      break;
    }
    node = node->parent();
  }
  return instanceNode;
}