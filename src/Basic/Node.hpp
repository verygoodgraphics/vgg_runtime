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
    return !m_parent.expired() ? m_parent.lock() : nullptr;
  }

  bool hasChild() const
  {
    return !m_firstChild.empty();
  }

  NodePtr root() const;

  void pushChildBack(NodePtr node);

  void pushChildFront(NodePtr node);

  void pushChildAt(const std::string& name, NodePtr node);

  void pushSiblingFront(NodePtr node);

  void pushSiblingAt(const std::string& name, NodePtr node);

  void pushSiblingBack(NodePtr node);

  NodePtr removeChild(const std::string& name);

  NodePtr removeSibling(const std::string& name);

  NodePtr findChild(const std::string& name) const;

  NodePtr findNextSblingFromCurrent(const std::string& name) const;

  NodePtr findPrevSiblingFromCurrent(const std::string& name) const;

  NodePtr findSibling(const std::string& name);

  NodePtr findChildRecursive(const std::string& name) const;

  auto begin()
  {
    return m_firstChild.begin();
  }

  auto end()
  {
    return m_firstChild.end();
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

  NodePtr _findChildRecursive(const NodePtr& ptr, const std::string& name) const;

protected:
  Node(const std::string& name)
    : m_name(name)
    , iter(m_firstChild.end())
  {
  }
};
} // namespace VGG
  //
