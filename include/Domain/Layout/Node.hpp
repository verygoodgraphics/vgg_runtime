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

#include "Domain/JsonDocument.hpp"
#include "Rect.hpp"

#include <memory>
#include <string>
#include <vector>
#include <functional>

class flexbox_node;
class grid_layout;

namespace VGG
{
namespace Layout
{
namespace Internal
{
struct AutoLayout;

namespace Rule
{
struct Rule;
}

} // namespace Internal
} // namespace Layout

class LayoutNode : public std::enable_shared_from_this<LayoutNode>
{
  std::weak_ptr<LayoutNode> m_parent;
  std::vector<std::shared_ptr<LayoutNode>> m_children;

  std::weak_ptr<JsonDocument> m_viewModel;
  const std::string m_path;
  Layout::Rect m_frame;
  Layout::Rect m_bounds;

  std::shared_ptr<Layout::Internal::AutoLayout> m_autoLayout;
  bool m_needsLayout{ false };

public:
  using HitTestHook = std::function<bool(const std::string&)>;

  LayoutNode(const std::string& path, const Layout::Rect& frame, const Layout::Rect& bounds)
    : m_path{ path }
    , m_frame{ frame }
    , m_bounds{ bounds }
  {
  }

  std::shared_ptr<LayoutNode> hitTest(const Layout::Point& point,
                                      const HitTestHook& hasEventListener)
  {
    // test front child first
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
    {
      if ((*it)->pointInside(point))
      {
        if (auto targetNode = (*it)->hitTest(point, hasEventListener))
        {
          return targetNode;
        }
      }
    }

    if (pointInside(point))
    {
      if (hasEventListener(m_path))
      {
        return shared_from_this();
      }
    }

    return nullptr;
  }

  void addChild(std::shared_ptr<LayoutNode> child)
  {
    if (!child)
    {
      return;
    }

    child->m_parent = weak_from_this();
    m_children.push_back(child);
  }

  const std::shared_ptr<LayoutNode> parent() const
  {
    return m_parent.lock();
  }

  const std::vector<std::shared_ptr<LayoutNode>>& children() const
  {
    return m_children;
  }

  const std::string& path() const
  {
    return m_path;
  }

  const Layout::Rect& frame() const;
  const Layout::Rect& bounds() const;
  void setFrame(const Layout::Rect& frame, bool updateRule = false);
  void setViewModel(JsonDocumentPtr viewModel);

public:
  std::shared_ptr<Layout::Internal::AutoLayout> autoLayout() const;
  std::shared_ptr<Layout::Internal::AutoLayout> createAutoLayout();
  void configureAutoLayout();

  std::shared_ptr<LayoutNode> autoLayoutContainer();

  void setNeedLayout();
  void layoutIfNeeded();

  std::shared_ptr<LayoutNode> findDescendantNodeById(const std::string& id);

  Layout::Rect frameToAncestor(std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    return convertRectToAncestor(m_frame, ancestorNode);
  }

private:
  std::string id();

  bool pointInside(Layout::Point point)
  {
    auto rect = convertRectToAncestor(m_frame);
    return rect.contains(point);
  }

  Layout::Rect convertRectToAncestor(Layout::Rect rect,
                                     std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    return { converPointToAncestor(rect.origin, ancestorNode), rect.size };
  }

  Layout::Point converPointToAncestor(Layout::Point point,
                                      std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    if (ancestorNode.get() == this)
    {
      return point;
    }

    auto x = point.x;
    auto y = point.y;

    auto parent = m_parent.lock();
    while (parent != ancestorNode)
    {
      x += parent->m_frame.origin.x;
      y += parent->m_frame.origin.y;
      parent = parent->m_parent.lock();
    }

    return { x, y };
  }

  void scaleChildNodes(const Layout::Size& oldSize, const Layout::Size& newSize);
  void scaleContour(float xScaleFactor, float yScaleFactor);
  void scalePoint(nlohmann::json& json, const char* key, float xScaleFactor, float yScaleFactor);
};

} // namespace VGG
