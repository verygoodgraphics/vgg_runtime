#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/Layout.hpp"

namespace VGG
{
class StartRunning
{
  std::shared_ptr<Layout::Layout> m_layout;

public:
  StartRunning(std::shared_ptr<Daruma> model)
    : m_layout{ new Layout::Layout{ model } }
  {
  }

  void layout(Layout::Size size)
  {
    m_layout->layout(size);
  }

  nlohmann::json designDoc()
  {
    return m_layout->designDoc();
  }

  std::shared_ptr<LayoutView> layoutTree()
  {
    return m_layout->createLayoutTree();
  }
};

} // namespace VGG