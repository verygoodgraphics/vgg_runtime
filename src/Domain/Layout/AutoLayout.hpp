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
public:
  std::weak_ptr<LayoutView> view;
  std::shared_ptr<Rule::Rule> rule;

  VGG::Layout::Rect frame;
  std::shared_ptr<flexbox_node> flexNode;
  std::shared_ptr<grid_layout> gridNode;

  bool isIncludedInLayout{ true };

public:
  void applyLayout(bool preservingOrigin);
  void frameChanged();

  bool isLeaf();
  bool isEnabled()
  {
    return rule != nullptr;
  }

private:
  Size calculateLayout(Size size);
};

} // namespace Internal
} // namespace Layout
} // namespace VGG