#pragma once

#include "Rect.hpp"
#include "Rule.hpp"

#include <flexbox_node.h>
#include <grid_layout.h>

#include <memory>

namespace VGG
{
class LayoutNode;

namespace Layout
{

namespace Internal
{

namespace Rule
{
struct GridLayout;
}

class AutoLayout
{
private:
  std::unique_ptr<flexbox_node> m_flexNode;
  flexbox_node* m_flexNodePtr{ nullptr };

  std::unique_ptr<grid_layout> m_gridContainer;
  grid_layout* m_gridContainerPtr{ nullptr };

  std::shared_ptr<grid_item> m_gridItem;
  decltype(m_gridContainerPtr->calc_layout(-1, -1)) m_gridItemFrames;

  bool m_isContainer{ false };
  Rect m_frame;

public:
  std::weak_ptr<LayoutNode> view;
  std::weak_ptr<Rule::Rule> rule;

  bool isIncludedInLayout{ true };

public:
  void configure();
  void applyLayout(bool preservingOrigin);
  void setFrame(Rect frame);
  void updateSizeRule();

  void setNeedsLayout();

  bool isLeaf();
  bool isEnabled()
  {
    return !rule.expired();
  }
  bool isContainer()
  {
    return m_isContainer;
  }

  flexbox_node* getFlexContainer()
  {
    return getFlexNode();
  }

  std::unique_ptr<flexbox_node>& takeFlexItem()
  {
    return m_flexNode;
  }
  flexbox_node* getFlexItem()
  {
    return getFlexNode();
  }

  grid_layout* getGridContainer()
  {
    return m_gridContainerPtr;
  }
  std::shared_ptr<grid_item>& getGridItem()
  {
    return m_gridItem;
  }
  auto& gridItemFrames()
  {
    return m_gridItemFrames;
  }

private:
  Size calculateLayout(Size size);
  void configureFlexNodeSize(flexbox_node* node);
  void configureGridItemSize();

  flexbox_node* createFlexContainer();
  flexbox_node* createFlexItem();
  flexbox_node* createFlexNode();

  void configureFlexContainer(Rule::FlexboxLayout* layout);
  void configureFlexItem(Rule::FlexboxItem* layout);

  void configureGridContainer(Rule::GridLayout* layout);
  void configureGridItem(Rule::GridItem* layout);

  void resetFlexNode();
  void resetGridContainer();
  void resetGridItem();

  flexbox_node* getFlexNode()
  {
    return m_flexNodePtr;
  }
};

} // namespace Internal
} // namespace Layout
} // namespace VGG