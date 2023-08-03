#include "Core/Node.h"

namespace VGG
{

NodePtr Node::root() const
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

void Node::pushChildBack(NodePtr node)
{
  auto it = m_firstChild.insert(m_firstChild.end(), node);
  node->m_iter = it;
  node->m_parent = shared_from_this();
}

void Node::pushChildFront(NodePtr node)
{
  auto it = m_firstChild.insert(m_firstChild.begin(), node);
  node->m_iter = it;
  node->m_parent = shared_from_this();
}

void Node::pushChildAt(const std::string& name, NodePtr node)
{
  auto it = std::find_if(m_firstChild.begin(),
                         m_firstChild.end(),
                         [&name](const auto& a) { return a->m_name == name; });
  if (it != m_firstChild.end())
  {
    node->m_iter = m_firstChild.insert(it, node);
    node->m_parent = shared_from_this();
  }
}

void Node::pushSiblingFront(NodePtr node)
{
  auto p = m_parent.lock();
  if (p)
  {
    p->m_firstChild.push_front(node);
    node->m_parent = p;
  }
}

void Node::pushSiblingAt(const std::string& name, NodePtr node)
{
  auto p = m_parent.lock();
  if (p)
  {
    auto it = std::find_if(p->m_firstChild.begin(),
                           p->m_firstChild.end(),
                           [&name](const auto& a) { return a->m_name == name; });
    if (it != p->m_firstChild.end())
    {
      node->m_iter = p->m_firstChild.insert(it, node);
      node->m_parent = p;
    }
  }
}

void Node::pushSiblingBack(NodePtr node)
{
  auto p = m_parent.lock();
  if (p)
  {
    node->m_iter = p->m_firstChild.insert(p->m_firstChild.end(), node);
    node->m_parent = p;
  }
}

NodePtr Node::removeChild(const std::string& name)
{
  auto it = _findChild(name);
  if (it == m_firstChild.end())
  {
    return nullptr;
  }
  auto r = *it;
  r->m_parent.reset();
  r->m_iter = m_firstChild.end();
  m_firstChild.erase(it);
  return r;
}

NodePtr Node::removeSibling(const std::string& name)
{
  return nullptr;
}

NodePtr Node::findChild(const std::string& name) const
{
  auto it = _findChild(name);
  if (it != m_firstChild.end())
  {
    return *it;
  }
  return nullptr;
}

NodePtr Node::findNextSblingFromCurrent(const std::string& name) const
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

NodePtr Node::findPrevSiblingFromCurrent(const std::string& name) const
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

NodePtr Node::findSibling(const std::string& name)
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

NodePtr Node::findChildRecursive(const std::string& name) const
{
  for (const auto& a : m_firstChild)
  {
    if (a->m_name == name)
    {
      return a;
    }
    else
    {
      auto r = _findChildRecursive(a, name);
      if (r)
        return r;
    }
  }
  return nullptr;
}

NodePtr Node::_findChildRecursive(const NodePtr& ptr, const std::string& name) const
{
  for (const auto& a : ptr->m_firstChild)
  {
    if (a->m_name == name)
    {
      return a;
    }
    else
    {
      auto r = _findChildRecursive(a, name);
      if (r)
        return r;
    }
  }
  return nullptr;
}

NodePtr Node::clone() const
{
  return createNode(m_name);
}

NodePtr Node::cloneRecursive() const
{
  auto newNode = clone();
  if (newNode)
  {
    for (const auto& n : m_firstChild)
    {
      newNode->pushChildBack(n->cloneRecursive());
    }
  }
  return newNode;
}

} // namespace VGG
