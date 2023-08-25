#pragma once

#include "Rect.hpp"
#include "Rule.hpp"

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

struct AutoLayout
{
  std::weak_ptr<LayoutView> view;
  std::shared_ptr<Rule::Rule> rule;

  std::shared_ptr<flexbox_node> flexNode;
  std::shared_ptr<grid_layout> gridNode;

  bool isEnabled{ false };
  bool isIncludedInLayout{ true };

  void applyLayout(bool preservingOrigin);

  bool isLeaf();

private:
  Size calculateLayout(Size size);
};

} // namespace Internal
} // namespace Layout
} // namespace VGG