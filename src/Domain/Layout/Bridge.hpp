#pragma once

#include "flexbox_node.h"
#include "grid_layout.h"

#include <memory>

namespace VGG
{
class LayoutView;

namespace Layout
{

namespace Internal
{

struct Bridge
{
  std::weak_ptr<LayoutView> view;
  std::shared_ptr<flexbox_node> flex_node;
  std::shared_ptr<grid_layout> grid_node;
};

} // namespace Internal
} // namespace Layout
} // namespace VGG