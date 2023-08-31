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
private:
  std::unique_ptr<flexbox_node> m_flexNode;
  std::unique_ptr<grid_layout> m_gridNode;

  flexbox_node* m_flexNodePtr;
  grid_layout* m_gridNodePtr;

public:
  std::weak_ptr<LayoutView> view;
  std::weak_ptr<Rule::Rule> rule;

  VGG::Layout::Rect frame;

  bool isIncludedInLayout{ true };

public:
  void configure();
  void applyLayout(bool preservingOrigin);
  void frameChanged();

  bool isLeaf();
  bool isEnabled()
  {
    return !rule.expired();
  }

  std::unique_ptr<flexbox_node>& takeFlexNode();
  std::unique_ptr<grid_layout>& takeGridNode();

  flexbox_node* createFlexNode();
  grid_layout* createGridNode();

  flexbox_node* getFlexNode();
  grid_layout* getGridNode();

private:
  Size calculateLayout(Size size);
  void configureNode(flexbox_node* node, std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule);
};

} // namespace Internal
} // namespace Layout
} // namespace VGG