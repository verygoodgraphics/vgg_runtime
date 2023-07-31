#pragma once
#include "Common/Config.h"
#include <algorithm>
#include <string>
#include <memory>
#include <list>
#include <utility>

#include <memory>

template<typename T>
static inline T* GetImplPtrHelper(T* ptr)
{
  return ptr;
}
template<typename Wrapper>
static inline typename Wrapper::pointer GetImplPtrHelper(const Wrapper& p)
{
  return p.data();
}

#define VGG_DECL_IMPL(Class)                                                                       \
  inline Class##__pImpl* d_func()                                                                  \
  {                                                                                                \
    return reinterpret_cast<Class##__pImpl*>(GetImplPtrHelper(d_ptr.get()));                       \
  }                                                                                                \
  inline const Class##__pImpl* d_func() const                                                      \
  {                                                                                                \
    return reinterpret_cast<const Class##__pImpl*>(GetImplPtrHelper(d_ptr.get()));                 \
  }                                                                                                \
  friend class Class##__pImpl;                                                                     \
  std::unique_ptr<Class##__pImpl> const d_ptr;

#define VGG_DECL_API(Class)                                                                        \
  inline Class* q_func()                                                                           \
  {                                                                                                \
    return static_cast<Class*>(q_ptr);                                                             \
  }                                                                                                \
  inline const Class* q_func() const                                                               \
  {                                                                                                \
    return static_cast<const Class*>(q_ptr);                                                       \
  }                                                                                                \
  friend class Class;                                                                              \
  Class* const q_ptr = nullptr;

#define VGG_IMPL(Class) Class##__pImpl* const _ = d_func();
#define VGG_API (Class) Class* const _ = q_func();

#define VGG_CALL __cdecl

#define VGG_EXPORTS_C_BEGIN                                                                        \
  extern "C"                                                                                       \
  {
#define VGG_EXPORTS_C_END }
#define VGG_EXPORTS_C(EXPR_OR_FUNC) extern "C" EXPR_OR_FUNC

#define VGG_INLINE_DECL inline

namespace VGG
{
class Node;
using NodePtr = std::shared_ptr<Node>;
using NodeRef = std::weak_ptr<Node>;

class VGG_EXPORTS Node : public std::enable_shared_from_this<Node>
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

  virtual NodePtr clone() const
  {
    auto newNode = createNode(m_name);
    for (const auto& n : m_firstChild)
    {
      newNode->pushChildFront(n->clone());
    }
    return newNode;
  }

  virtual ~Node() = default;

protected:
  std::string m_name;
  FirstChildNode m_firstChild;
  std::list<std::shared_ptr<Node>>::iterator m_iter;
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
    , m_iter(m_firstChild.end())
  {
  }
};
} // namespace VGG
  //
