#pragma once
#include <algorithm>
#include <string>
#include <memory>
#include <list>
#include <utility>

namespace VGG
{
class Node;
using NodePtr = std::shared_ptr<Node>;
using NodeRef = std::weak_ptr<Node>;

class Node : public std::enable_shared_from_this<Node>
{
  using FirstChildNode = std::list<std::shared_ptr<Node>>;

public:
  struct Iterator
  {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = NodePtr;
    using pointer = NodePtr*;
    using reference = NodePtr&;

    Iterator(pointer ptr)
      : m_ptr(ptr)
    {
    }

    reference operator*() const
    {
      return *m_ptr;
    }
    pointer operator->()
    {
      return m_ptr;
    }
    Iterator& operator++()
    {
      m_ptr++;
      return *this;
    }
    Iterator operator++(int)
    {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    friend bool operator==(const Iterator& a, const Iterator& b)
    {
      return a.m_ptr == b.m_ptr;
    };
    friend bool operator!=(const Iterator& a, const Iterator& b)
    {
      return a.m_ptr != b.m_ptr;
    };

  private:
    pointer m_ptr;
  };

  const std::string& getName() const
  {
    return m_name;
  }

  NodePtr parent() const
  {
    return m_parent.lock();
  }

  NodePtr root() const
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

  void pushChildBack(NodePtr node)
  {
    auto it = m_firstChild.insert(m_firstChild.end(), node);
    node->iter = it;
    node->m_parent = shared_from_this();
  }

  void pushChildFront(NodePtr node)
  {
    auto it = m_firstChild.insert(m_firstChild.begin(), node);
    node->iter = it;
    node->m_parent = shared_from_this();
  }

  void pushChildAt(const std::string& name, NodePtr node)
  {
    auto it = std::find_if(m_firstChild.begin(),
                           m_firstChild.end(),
                           [&name](const auto& a) { return a->m_name == name; });
    if (it != m_firstChild.end())
    {
      node->iter = m_firstChild.insert(it, node);
      node->m_parent = shared_from_this();
    }
  }

  void pushSiblingFront(NodePtr node)
  {
    auto p = m_parent.lock();
    if (p)
    {
      p->m_firstChild.push_front(node);
      node->m_parent = p;
    }
  }

  void pushSiblingAt(const std::string& name, NodePtr node)
  {
    auto p = m_parent.lock();
    if (p)
    {
      auto it = std::find_if(p->m_firstChild.begin(),
                             p->m_firstChild.end(),
                             [&name](const auto& a) { return a->m_name == name; });
      if (it != p->m_firstChild.end())
      {
        node->iter = p->m_firstChild.insert(it, node);
        node->m_parent = p;
      }
    }
  }

  void pushSiblingBack(NodePtr node)
  {
    auto p = m_parent.lock();
    if (p)
    {
      node->iter = p->m_firstChild.insert(p->m_firstChild.end(), node);
      node->m_parent = p;
    }
  }

  NodePtr removeChild(const std::string& name)
  {
    auto it = _findChild(name);
    if (it == m_firstChild.end())
    {
      return nullptr;
    }
    auto r = *it;
    r->m_parent.reset();
    r->iter = m_firstChild.end();
    m_firstChild.erase(it);
    return r;
  }

  NodePtr removeSibling(const std::string& name)
  {
    return nullptr;
  }

  NodePtr findChild(const std::string& name) const
  {
    auto it = _findChild(name);
    if (it != m_firstChild.end())
    {
      return *it;
    }
    return nullptr;
  }

  NodePtr findNextSblingFromCurrent(const std::string& name) const
  {
    auto p = m_parent.lock();
    if (!p)
    {
      return nullptr;
    }
    auto it =
      std::find_if(iter, p->m_firstChild.end(), [&name](auto& a) { return a->m_name == name; });
    if (it != p->m_firstChild.end())
    {
      return *it;
    }
    return nullptr;
  }

  NodePtr findPrevSiblingFromCurrent(const std::string& name) const
  {
    auto p = m_parent.lock();
    if (!p)
    {
      return nullptr;
    }
    auto it =
      std::find_if(p->m_firstChild.begin(), iter, [&name](auto& a) { return a->m_name == name; });
    if (it != p->m_firstChild.end())
    {
      return *it;
    }
    return nullptr;
  }

  NodePtr findSibling(const std::string& name)
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

  NodePtr findChildRecursive(const std::string& name) const
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

  NodePtr _findChildRecursive(const NodePtr& ptr, const std::string& name) const
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

  // used for rendering
  virtual void traverse()
  {
    preVisit();
    for (const auto& p : this->m_firstChild)
    {
      p->traverse();
    }
    postVisit();
  }

  static NodePtr createNode(const std::string& name)
  {
    return NodePtr(new Node(name));
  }

  virtual ~Node() = default;

protected:
  std::string m_name;
  FirstChildNode m_firstChild;
  std::list<std::shared_ptr<Node>>::iterator iter;
  NodeRef m_parent;
  std::list<std::shared_ptr<Node>>::const_iterator _findChild(const std::string& name) const
  {
    return std::find_if(m_firstChild.begin(),
                        m_firstChild.end(),
                        [&name](const auto& a) { return a->m_name == name; });
  }

protected:
  Node(const std::string& name)
    : m_name(name)
    , iter(m_firstChild.end())
  {
  }

  virtual void postVisit()
  {
  }

  virtual void preVisit()
  {
  }
};
} // namespace VGG
  //
