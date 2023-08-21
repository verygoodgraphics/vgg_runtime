#pragma once

#include "Domain/Layout/View.hpp"
#include "Domain/Daruma.hpp"

namespace VGG
{

class ResizeWindow
{
public:
  void onResize(std::shared_ptr<Daruma> model,
                std::shared_ptr<LayoutView> layoutTree,
                Layout::Size newSize);
};

} // namespace VGG