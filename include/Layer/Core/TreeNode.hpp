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

#include "Layer/Core/VNode.hpp"
#include "Layer/Config.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Memory/VRefCnt.hpp"
#include <algorithm>
#include <string>
#include <memory>
#include <list>
#include <utility>
#include <memory>

namespace VGG
{
class TreeNode;

#ifdef USE_SHARED_PTR
using TreeNodePtr = std::shared_ptr<TreeNode>;
using TreeNodeRef = std::weak_ptr<TreeNode>;
#else
using TreeNodePtr = VGG::layer::Ref<TreeNode>;
using TreeNodeRef = VGG::layer::WeakRef<TreeNode>;

#endif

template<typename... Args>
inline TreeNodePtr makeTreeNodePtr(Args&&... args)
{
#ifdef USE_SHARED_PTR
  auto p = std::make_shared<TreeNode>(std::forward<Args>(args)...);
  return p;
#else
  return TreeNodePtr(layer::V_NEW<TreeNode>(std::forward<Args>(args)...));
#endif
};
class VGG_EXPORTS TreeNode : public layer::VNode
{
  using FirstChildNode = std::list<TreeNodePtr>;

public:
  using NodeIter = std::list<TreeNodePtr>::iterator;
  TreeNode(layer::VRefCnt* cnt)
    : VNode(cnt)
  {
  }
  TreeNode(TreeNode&) = delete;
  TreeNode& operator=(const TreeNode& other) = delete;
  TreeNode(TreeNode&&) noexcept = delete;
  TreeNode& operator=(TreeNode&&) noexcept = delete;
  struct Iterator
  {
    // NOLINTBEGIN
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = TreeNodePtr;
    using pointer = TreeNodePtr*;
    using reference = TreeNodePtr&;
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

  TreeNodePtr parent() const
  {
    return !m_parent.expired() ? m_parent.lock() : nullptr;
  }

  bool hasChild() const
  {
    return !m_firstChild.empty();
  }

  TreeNodePtr root() const;

  NodeIter iter()
  {
    return m_iter;
  }

  void pushChildBack(TreeNodePtr node);

  void pushChildFront(TreeNodePtr node);

  void pushChildAt(const std::string& name, TreeNodePtr node);

  void pushSiblingFront(TreeNodePtr node);

  void pushSiblingAt(const std::string& name, TreeNodePtr node);

  void pushSiblingBack(TreeNodePtr node);

  TreeNodePtr removeChild(const std::string& name);

  TreeNodePtr removeSibling(const std::string& name);

  TreeNodePtr findChild(const std::string& name) const;

  TreeNodePtr findNextSblingFromCurrent(const std::string& name) const;

  TreeNodePtr findPrevSiblingFromCurrent(const std::string& name) const;

  TreeNodePtr findSibling(const std::string& name);

  TreeNodePtr findChildRecursive(const std::string& name) const;

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

  static TreeNodePtr createNode(const std::string& name)
  {
#ifdef USE_SHARED_PTR
    return TreeNodePtr(new TreeNode(name));
#else
    return nullptr;
    // TreeNodePtr();
    //  makeTreeNodePtr(name);
#endif
  }

  virtual TreeNodePtr clone() const;

  TreeNodePtr cloneChildren() const;

  virtual ~TreeNode();

protected:
  std::string                            m_name;
  FirstChildNode                         m_firstChild;
  std::list<TreeNodePtr>::iterator       m_iter;
  TreeNodeRef                            m_parent;
  std::list<TreeNodePtr>::const_iterator findChildImpl(const std::string& name) const
  {
    return std::find_if(
      m_firstChild.begin(),
      m_firstChild.end(),
      [&name](const auto& a) { return a->m_name == name; });
  }

  TreeNodePtr findChildRecursiveImpl(const TreeNodePtr& ptr, const std::string& name) const;

  Bound onRevalidate() override
  {
    return Bound();
  }

protected:
  TreeNode(layer::VRefCnt* cnt, const std::string& name)
    : VNode(cnt)
    , m_name(name)
    , m_iter(m_firstChild.end())
  {
  }
};
} // namespace VGG
  //
