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
    return nullptr;
  }

  bool pointInside(Layout::Point point)
  {
    return false;
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
};

} // namespace VGG