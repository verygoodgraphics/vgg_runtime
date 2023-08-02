#pragma once

#include "Rect.hpp"

#include <memory>
#include <string>
#include <vector>

namespace VGG
{

class LayoutView : public std::enable_shared_from_this<LayoutView>
{
  std::shared_ptr<LayoutView> m_parent;
  std::vector<std::shared_ptr<LayoutView>> m_children;

  const std::string m_path;
  Layout::Rect m_frame;

public:
  LayoutView(const std::string& path, const Layout::Rect& frame)
    : m_path{ path }
    , m_frame{ frame }
  {
  }

  std::shared_ptr<LayoutView> hitTest(Layout::Point point)
  {
    // test top z-index child first
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
    {
      if ((*it)->pointInside(point))
      {
        return (*it)->hitTest(point);
      }
    }

    if (pointInside(point))
    {
      return shared_from_this();
    }
    else
    {
      return nullptr;
    }
  }

  bool pointInside(Layout::Point point)
  {
    auto rect = convertRectToWindow(m_frame);
    return rect.pointInRect(point);
  }

  auto frame()
  {
    return m_frame;
  }

  std::shared_ptr<LayoutView> parent()
  {
    return m_parent;
  }
  void setParent(std::shared_ptr<LayoutView> parent)
  {
    m_parent = parent;
  }

  void addChild(std::shared_ptr<LayoutView> child)
  {
    if (!child)
    {
      return;
    }

    child->setParent(shared_from_this());
    m_children.push_back(child);
  }

private:
  Layout::Rect convertRectToWindow(Layout::Rect rect)
  {
    return { converPointToWindow(rect.origin), rect.size };
  }

  Layout::Point converPointToWindow(Layout::Point point)
  {
    auto x = point.x;
    auto y = point.y;

    auto parent = m_parent;
    while (parent)
    {
      x += parent->frame().origin.x;
      y += parent->frame().origin.y;
      parent = parent->parent();
    }

    return { x, y };
  }
};

} // namespace VGG