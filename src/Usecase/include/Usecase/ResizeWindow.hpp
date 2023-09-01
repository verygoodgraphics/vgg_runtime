#pragma once

#include "Domain/Layout/Layout.hpp"
#include "Domain/Layout/Node.hpp"
#include "Domain/Daruma.hpp"

namespace VGG
{

class ResizeWindow
{
  std::shared_ptr<Layout::Layout> m_layout;

public:
  ResizeWindow(std::shared_ptr<Layout::Layout> layout)
    : m_layout{ layout }
  {
  }

  void onResize(Layout::Size newSize);
};

} // namespace VGG