/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <vector>
#include "Rect.hpp"
#include "Rule.hpp"
#include <flexbox_node.h>
#include <grid_layout.h>
class grid_item;

namespace VGG
{
class LayoutNode;

namespace Layout
{

namespace Internal
{

class AutoLayout
{
private:
  std::unique_ptr<flexbox_node> m_flexNode;
  flexbox_node*                 m_flexNodePtr{ nullptr };
  std::size_t                   m_flexNodeIndex{ std::numeric_limits<std::size_t>::max() };

  std::unique_ptr<grid_layout> m_gridContainer;
  grid_layout*                 m_gridContainerPtr{ nullptr };

  std::shared_ptr<grid_item>                        m_gridItem;
  decltype(m_gridContainerPtr->calc_layout(-1, -1)) m_gridItemFrames;

  Rect m_frame;

  bool m_hasUnkownWidthChild{ false };
  bool m_hasUnknownHeightChild{ false };
  bool m_hasFixedWidthChild{ false };
  bool m_hasFixedHeightChild{ false };

public:
  std::weak_ptr<LayoutNode> view;
  std::weak_ptr<Rule::Rule> rule;

public:
  void configure();
  void configureFlexContainer();
  void configureFlexItemMargin();
  void isFixedSize(bool& outWidth, bool& outHeight);
  void applyLayout(bool preservingOrigin);
  void setFrame(Rect frame);
  void updateSizeRule(Size newSize);

  std::shared_ptr<LayoutNode> setNeedsLayout(); // return node that needs layout

  void removeSubtree();

  bool isIncludedInLayout();
  bool isAboslutePosition();

  bool isLeaf();
  bool isEnabled()
  {
    return !rule.expired();
  }
  bool isContainer();
  bool isFlexOrGridItem();

  flexbox_node* getOrCreateFlexContainer();
  flexbox_node* getFlexContainer()
  {
    return isFlexContainer() ? getFlexNode() : nullptr;
  }

  std::unique_ptr<flexbox_node>& takeFlexItem()
  {
    return m_flexNode;
  }
  flexbox_node* getFlexItem()
  {
    return getFlexNode();
  }
  void setFlexNodeIndex(std::size_t index);
  void resetFlexNode();
  void takeFlexNodeFromTree();

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

public:
  void setHasUnknownWidthChild(const bool has)
  {
    m_hasUnkownWidthChild = has;
  }
  void setHasUnknownHeightChild(const bool has)
  {
    m_hasUnknownHeightChild = has;
  }
  void setHasFixedWidthChild(const bool has)
  {
    m_hasFixedWidthChild = has;
  }
  void setHasFixedHeightChild(const bool has)
  {
    m_hasFixedHeightChild = has;
  }

private:
  Size calculateLayout(Size size);
  void configureFlexNodeSize(flexbox_node* node, bool forContainer = false);
  void configureGridItemSize();

  flexbox_node* createFlexContainer();
  flexbox_node* createFlexItem();
  flexbox_node* createFlexNode();

  flexbox_node* parentFlexContainer();

  void configureFlexContainer(Rule::FlexboxLayout* layout);
  void configureFlexItem(Rule::FlexboxItem* layout);
  void configureFlexItemAlignSelf(flexbox_node* node);
  bool isHorizontalDirection();
  bool isVerticalDirection();
  bool directionIs(Rule::FlexboxLayout::EDirection direction);
  bool is100PercentWidth();
  bool is100PercentHeight();
  bool isFitContentWidth();
  bool isFitContentHeight();
  void isAsFixedSize(bool& inOutWidth, bool& inOutHeight);
  bool isSmartSpacingAndFlexSpaceBetweenAndNoWrap();
  bool isEmptyContainer();
  bool isOnlyChild();
  bool isFirstChild();
  bool isLastChild();

  bool isFlexContainer();
  bool isGridContainer();

  bool shouldChangeContainerHugWidth();
  bool shouldChangeContainerHugHeight();

  Rule::GridLayout* gridLayout();

  void configureGridContainer();
  void configureGridItem(Rule::GridItem* layout);

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
