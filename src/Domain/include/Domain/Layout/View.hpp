#pragma once

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
struct Bridge;

} // namespace Internal
} // namespace Layout

class LayoutView : public std::enable_shared_from_this<LayoutView>
{
  std::weak_ptr<LayoutView> m_parent;
  std::vector<std::shared_ptr<LayoutView>> m_children;

  const std::string m_path;
  Layout::Rect m_frame;

  std::shared_ptr<Layout::Internal::Bridge> m_bridge;

public:
  using HitTestHook = std::function<bool(const std::string&)>;

  LayoutView(const std::string& path, const Layout::Rect& frame)
    : m_path{ path }
    , m_frame{ frame }
  {
  }

  std::shared_ptr<LayoutView> hitTest(const Layout::Point& point,
                                      const HitTestHook& hasEventListener)
  {
    // test front child first
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
    {
      if ((*it)->pointInside(point))
      {
        if (auto target_view = (*it)->hitTest(point, hasEventListener))
        {
          return target_view;
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

  void addChild(std::shared_ptr<LayoutView> child)
  {
    if (!child)
    {
      return;
    }

    child->m_parent = weak_from_this();
    m_children.push_back(child);
  }

  const std::vector<std::shared_ptr<LayoutView>>& children() const
  {
    return m_children;
  }

  const std::string& path() const
  {
    return m_path;
  }

  const auto& frame() const
  {
    return m_frame;
  }

  void setFrame(const Layout::Rect& frame)
  {
    bool needLayout = m_frame.size != frame.size;
    m_frame = frame;

    if (needLayout)
    {
      // todo, scale subview
      // todo, update json model
      applyLayout();
    }
  }

public:
  std::shared_ptr<Layout::Internal::Bridge> configureLayout();
  std::shared_ptr<flexbox_node> createFlexboxNode();
  std::shared_ptr<grid_layout> createGridNode();
  void applyLayout();

  std::shared_ptr<Layout::Internal::Bridge> layoutBridge() const
  {
    return m_bridge;
  }

private:
  bool pointInside(Layout::Point point)
  {
    auto rect = convertRectToWindow(m_frame);
    return rect.pointInRect(point);
  }

  Layout::Rect convertRectToWindow(Layout::Rect rect)
  {
    return { converPointToWindow(rect.origin), rect.size };
  }

  Layout::Point converPointToWindow(Layout::Point point)
  {
    auto x = point.x;
    auto y = point.y;

    auto parent = m_parent.lock();
    while (parent)
    {
      x += parent->m_frame.origin.x;
      y += parent->m_frame.origin.y;
      parent = parent->m_parent.lock();
    }

    return { x, y };
  }
};

} // namespace VGG
