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
#include "Layer/Core/TreeNode.hpp"
#include <memory>

namespace VGG
{

TreeNodePtr TreeNode::root() const
{
  auto p = parent();
  while (true)
  {
    if (!p->parent())
    {
      return p;
    }
    p = p->parent();
  }
  return nullptr;
}

void TreeNode::pushChildBack(TreeNodePtr node)
{
  auto it = m_firstChild.insert(m_firstChild.end(), node);
  node->m_iter = it;
  node->m_parent = std::static_pointer_cast<TreeNode>(shared_from_this());
  observe(node);
}

void TreeNode::pushChildFront(TreeNodePtr node)
{
  auto it = m_firstChild.insert(m_firstChild.begin(), node);
  node->m_iter = it;
  node->m_parent = std::static_pointer_cast<TreeNode>(shared_from_this());
  observe(node);
}

void TreeNode::pushChildAt(const std::string& name, TreeNodePtr node)
{
  auto it = std::find_if(
    m_firstChild.begin(),
    m_firstChild.end(),
    [&name](const auto& a) { return a->m_name == name; });
  if (it != m_firstChild.end())
  {
    node->m_iter = m_firstChild.insert(it, node);
    node->m_parent = std::static_pointer_cast<TreeNode>(shared_from_this());
    observe(node);
  }
}

void TreeNode::pushSiblingFront(TreeNodePtr node)
{
  auto p = m_parent.lock();
  if (p)
  {
    p->m_firstChild.push_front(node);
    node->m_parent = p;
    observe(node);
  }
}

void TreeNode::pushSiblingAt(const std::string& name, TreeNodePtr node)
{
  auto p = m_parent.lock();
  if (p)
  {
    auto it = std::find_if(
      p->m_firstChild.begin(),
      p->m_firstChild.end(),
      [&name](const auto& a) { return a->m_name == name; });
    if (it != p->m_firstChild.end())
    {
      node->m_iter = p->m_firstChild.insert(it, node);
      node->m_parent = p;
      observe(node);
    }
  }
}

void TreeNode::pushSiblingBack(TreeNodePtr node)
{
  auto p = m_parent.lock();
  if (p)
  {
    node->m_iter = p->m_firstChild.insert(p->m_firstChild.end(), node);
    node->m_parent = p;
    observe(node);
  }
}

TreeNodePtr TreeNode::removeChild(const std::string& name)
{
  auto it = findChildImpl(name);
  if (it == m_firstChild.end())
  {
    return nullptr;
  }
  auto r = *it;
  unobserve(*it);
  r->m_parent.reset();
  r->m_iter = m_firstChild.end();
  m_firstChild.erase(it);
  return r;
}

TreeNodePtr TreeNode::removeSibling(const std::string& name)
{
  return nullptr;
}

TreeNodePtr TreeNode::findChild(const std::string& name) const
{
  auto it = findChildImpl(name);
  if (it != m_firstChild.end())
  {
    return *it;
  }
  return nullptr;
}

TreeNodePtr TreeNode::findNextSblingFromCurrent(const std::string& name) const
{
  auto p = m_parent.lock();
  if (!p)
  {
    return nullptr;
  }
  auto it =
    std::find_if(m_iter, p->m_firstChild.end(), [&name](auto& a) { return a->m_name == name; });
  if (it != p->m_firstChild.end())
  {
    return *it;
  }
  return nullptr;
}

TreeNodePtr TreeNode::findPrevSiblingFromCurrent(const std::string& name) const
{
  auto p = m_parent.lock();
  if (!p)
  {
    return nullptr;
  }
  auto it =
    std::find_if(p->m_firstChild.begin(), m_iter, [&name](auto& a) { return a->m_name == name; });
  if (it != p->m_firstChild.end())
  {
    return *it;
  }
  return nullptr;
}

TreeNodePtr TreeNode::findSibling(const std::string& name)
{
  auto p = m_parent.lock();
  if (!p)
  {
    return nullptr;
  }
  for (const auto& a : p->m_firstChild)
  {
    if (a->m_name == name)
    {
      return a;
    }
  }
  return nullptr;
}

TreeNodePtr TreeNode::findChildRecursive(const std::string& name) const
{
  for (const auto& a : m_firstChild)
  {
    if (a->m_name == name)
    {
      return a;
    }
    else
    {
      auto r = findChildRecursiveImpl(a, name);
      if (r)
        return r;
    }
  }
  return nullptr;
}

TreeNodePtr TreeNode::findChildRecursiveImpl(const TreeNodePtr& ptr, const std::string& name) const
{
  for (const auto& a : ptr->m_firstChild)
  {
    if (a->m_name == name)
    {
      return a;
    }
    else
    {
      auto r = findChildRecursiveImpl(a, name);
      if (r)
        return r;
    }
  }
  return nullptr;
}

TreeNodePtr TreeNode::clone() const
{
  return createNode(m_name);
}

TreeNodePtr TreeNode::cloneChildren() const
{
  auto newNode = clone();
  if (newNode)
  {
    for (const auto& n : m_firstChild)
    {
      newNode->pushChildBack(n->cloneChildren());
    }
  }
  return newNode;
}

TreeNode::~TreeNode()
{
}

} // namespace VGG
