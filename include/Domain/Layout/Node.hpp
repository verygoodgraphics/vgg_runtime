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
  enum class EResizing
  {
    FIX_START_FIX_END,
    FIX_START_FIX_SIZE,
    FIX_START_SCALE,
    FIX_END_FIX_SIZE,
    FIX_END_SCALE,
    SCALE,
    FIX_CENTER_RATIO_FIX_SIZE,
    FIX_CENTER_OFFSET_FIX_SIZE
  };
  enum class EAdjustContentOnResize
  {
    SKIP_GROUP_OR_BOOLEAN_GROUP,
    DISABLED,
    ENABLED,
  };

  std::weak_ptr<LayoutNode> m_parent;
  std::vector<std::shared_ptr<LayoutNode>> m_children;

  std::weak_ptr<JsonDocument> m_viewModel;
  const std::string m_path;

  std::shared_ptr<Layout::Internal::AutoLayout> m_autoLayout;
  bool m_needsLayout{ false };
  Layout::Rect m_oldFrame;

public:
  using HitTestHook = std::function<bool(const std::string&)>;

  LayoutNode(const std::string& path)
    : m_path{ path }
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

  Layout::Rect frame() const;
  Layout::Rect bounds() const;
  void setFrame(const Layout::Rect& newFrame, bool updateRule = false, bool useOldFrame = false);
  void setViewModel(JsonDocumentPtr viewModel);

public:
  std::shared_ptr<Layout::Internal::AutoLayout> autoLayout() const;
  std::shared_ptr<Layout::Internal::AutoLayout> createAutoLayout();
  void configureAutoLayout();

  std::shared_ptr<LayoutNode> autoLayoutContainer();

  void setNeedLayout();
  void layoutIfNeeded();
  void scaleTo(const Layout::Size& newSize, bool updateRule);

  std::shared_ptr<LayoutNode> findDescendantNodeById(const std::string& id);

  Layout::Rect frameToAncestor(std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    return convertRectToAncestor(frame(), ancestorNode);
  }

private:
  std::string id();

  bool pointInside(Layout::Point point)
  {
    auto rect = convertRectToAncestor(frame());
    return rect.contains(point);
  }

  Layout::Rect convertRectToAncestor(Layout::Rect rect,
                                     std::shared_ptr<LayoutNode> ancestorNode = nullptr)
  {
    return { converPointToAncestor(rect.origin, ancestorNode), rect.size };
  }

  Layout::Point converPointToAncestor(Layout::Point point,
                                      std::shared_ptr<LayoutNode> ancestorNode = nullptr);

  void scaleChildNodes(const Layout::Size& oldContainerSize,
                       const Layout::Size& newContainerSize,
                       bool useOldFrame);
  void scaleContour(float xScaleFactor, float yScaleFactor);
  void scalePoint(nlohmann::json& json, const char* key, float xScaleFactor, float yScaleFactor);

  Layout::Point origin() const;
  Layout::Size size() const;
  void saveOldFrame();
  void saveChildrendOldFrame();

  void updateModel(const Layout::Rect& frame);
  Layout::Point modelOrigin() const;
  Layout::Rect modelBounds() const;

  Layout::Rect resize(const Layout::Size& oldContainerSize,
                      const Layout::Size& newContainerSize,
                      bool useOldFrame,
                      const Layout::Point* parentOrigin = nullptr,
                      bool dry = false);
  Layout::Rect resizeGroup(const Layout::Size& oldContainerSize,
                           const Layout::Size& newContainerSize,
                           bool useOldFrame);
  std::pair<Layout::Scalar, Layout::Scalar> resizeH(
    const Layout::Size& oldContainerSize,
    const Layout::Size& newContainerSize,
    bool useOldFrame,
    const Layout::Point* parentOrigin) const; // return x, w
  std::pair<Layout::Scalar, Layout::Scalar> resizeV(
    const Layout::Size& oldContainerSize,
    const Layout::Size& newContainerSize,
    bool useOldFrame,
    const Layout::Point* parentOrigin) const; // return y, h

  EResizing horizontalResizing() const;
  EResizing verticalResizing() const;
  EAdjustContentOnResize adjustContentOnResize() const;

  const nlohmann::json* model() const;

  template<typename T>
  T getValue(const char* key, T v) const;

  bool shouldSkip();
};

} // namespace VGG
