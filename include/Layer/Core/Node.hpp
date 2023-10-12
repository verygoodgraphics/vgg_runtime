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
#pragma once
#include "Utility/HelperMacro.hpp"
#include "Layer/Config.hpp"
#include <algorithm>
#include <string>
#include <memory>
#include <list>
#include <utility>
#include <memory>

namespace VGG
{
class Node;
using NodePtr = std::shared_ptr<Node>;
using NodeRef = std::weak_ptr<Node>;

class VGG_EXPORTS Node : public std::enable_shared_from_this<Node>
{
  using FirstChildNode = std::list<std::shared_ptr<Node>>;

public:
  using NodeIter = std::list<std::shared_ptr<Node>>::iterator;
  Node(const Node& other)
  {
    m_name = other.m_name;
  }
  Node& operator=(const Node& other) = delete;
  Node(Node&&) noexcept = default;
  Node& operator=(Node&&) noexcept = default;
  struct Iterator
  {
    // NOLINTBEGIN
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = NodePtr;
    using pointer = NodePtr*;
    using reference = NodePtr&;
    // NOLINTEND

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

  const std::string& name() const
  {
    return m_name;
  }

  void setName(const std::string& name)
  {
    m_name = name;
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

  NodeIter iter()
  {
    return m_iter;
  }

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

  auto cbegin() const
  {
    return m_firstChild.cbegin();
  }

  auto end()
  {
    return m_firstChild.end();
  }

  auto cend() const
  {
    return m_firstChild.cend();
  }

  static NodePtr createNode(const std::string& name)
  {
    return NodePtr(new Node(name));
  }

  virtual NodePtr clone() const;

  NodePtr cloneChildren() const;

  virtual ~Node() = default;

protected:
  std::string m_name;
  FirstChildNode m_firstChild;
  std::list<std::shared_ptr<Node>>::iterator m_iter;
  NodeRef m_parent;
  std::list<std::shared_ptr<Node>>::const_iterator findChildImpl(const std::string& name) const
  {
    return std::find_if(m_firstChild.begin(),
                        m_firstChild.end(),
                        [&name](const auto& a) { return a->m_name == name; });
  }

  NodePtr findChildRecursiveImpl(const NodePtr& ptr, const std::string& name) const;

protected:
  Node(const std::string& name)
    : m_name(name)
    , m_iter(m_firstChild.end())
  {
  }
};
} // namespace VGG
  //
