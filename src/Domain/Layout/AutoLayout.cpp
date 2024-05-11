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
#include "AutoLayout.hpp"

#include "Utility/Log.hpp"
#include "Rule.hpp"
#include "Node.hpp"

#include <memory>
#include <vector>

#undef DEBUG
#define DEBUG(msg, ...)

#define VERBOSE DEBUG
#undef VERBOSE
#define VERBOSE(msg, ...)

using namespace VGG::Layout::Internal::Rule;

namespace VGG
{
class LayoutNode;

namespace Layout
{

namespace Internal
{

namespace
{
using LengthTypes = VGG::Layout::Internal::Rule::Length::ETypes;
unit toLibUnit(LengthTypes type)
{
  switch (type)
  {
    case LengthTypes::PX:
      return unit_point;
    case LengthTypes::PERCENT:
      return unit_percent;
    case LengthTypes::AUTO:
    case LengthTypes::FIT_CONTENT:
      return unit_auto;
    default:
      return unit_undefine;
  }
}

position toLibPosition(Position::ETypes type)
{
  switch (type)
  {
    case Position::ETypes::RELATIVE:
      return position_relative;
    case Position::ETypes::ABSOLUTE:
      return position_absolute;

    default: // todo
      return position_absolute;
  }
}

direction toLibDirecion(FlexboxLayout::EDirection type)
{
  switch (type)
  {
    case FlexboxLayout::EDirection::HORIZONTAL:
      return direction_row;

    default:
      return direction_column;
  }
}

justify_content toLibJustifyContent(FlexboxLayout::EJustifyContent type)
{
  switch (type)
  {
    case FlexboxLayout::EJustifyContent::START:
      return justify_content_flex_start;
    case FlexboxLayout::EJustifyContent::CENTER:
      return justify_content_center;
    case FlexboxLayout::EJustifyContent::END:
      return justify_content_flex_end;
    case FlexboxLayout::EJustifyContent::SPACE_BETWEEN:
      return justify_content_space_between;
    case FlexboxLayout::EJustifyContent::SPACE_AROUND:
      return justify_content_space_around;
    case FlexboxLayout::EJustifyContent::SPACE_EVENLY:
      return justify_content_space_evenly;
    default:
      return justify_content_flex_start;
  }
}

align_content toLibAlignContent(EAlignStyle type)
{
  switch (type)
  {
    case EAlignStyle::START:
      return align_content_flex_start;
    case EAlignStyle::CENTER:
      return align_content_center;
    case EAlignStyle::END:
      return align_content_flex_end;
    case EAlignStyle::SPACE_BETWEEN:
      return align_content_space_between;
    case EAlignStyle::SPACE_AROUND:
      return align_content_space_around;
    case EAlignStyle::SPACE_EVENLY:
      return align_content_space_evenly;
    default:
      return align_content_flex_start;
  }
}

align_items toLibAlignItem(EAlignStyle type)
{
  switch (type)
  {
    case EAlignStyle::START:
      return align_items_flex_start;
    case EAlignStyle::CENTER:
      return align_items_center;
    case EAlignStyle::END:
      return align_items_flex_end;
    default:
      return align_items_flex_start;
  }
}

wrap toLibWrap(FlexboxLayout::EWrap type)
{
  switch (type)
  {
    case FlexboxLayout::EWrap::NO_WRAP:
      return wrap_no_wrap;
    case FlexboxLayout::EWrap::WRAP:
      return wrap_wrap;
    default:
      return wrap_no_wrap;
  }
}

column_width_strategy toLibColumnWidthStrategy(ColumnWidth::EStrategy type)
{
  switch (type)
  {
    case ColumnWidth::EStrategy::MIN:
      return column_width_strategy_min;
    case ColumnWidth::EStrategy::FIX:
      return column_width_strategy_fix;
    default:
      return column_width_strategy_min;
  }
}

row_height_strategy toLibRowHeightStrategy(RowHeight::EStrategy type)
{
  switch (type)
  {
    case RowHeight::EStrategy::FILL_CONTAINER:
      return row_height_strategy_fill;
    case RowHeight::EStrategy::FILL_CONTENT:
      return row_height_strategy_fit;
    case RowHeight::EStrategy::FIXED:
      return row_height_strategy_fix;
    default:
      return row_height_strategy_fill;
  }
}

align toLibAlign(EAlignStyle type)
{
  switch (type)
  {
    case EAlignStyle::START:
      return align_start;
    case EAlignStyle::CENTER:
      return align_center;
    case EAlignStyle::END:
      return align_end;
    default:
      return align_start;
  }
}

item_pos_strategy toLibItemPosStrategy(GridItemPos::EStrategy type)
{
  switch (type)
  {
    case GridItemPos::EStrategy::AUTO:
      return item_pos_strategy_auto;
    case GridItemPos::EStrategy::FIX:
      return item_pos_strategy_fix;
    default:
      return item_pos_strategy_auto;
  }
}

length_unit toLibLengthUnit(Length::ETypes type)
{
  switch (type)
  {
    case Length::ETypes::PX:
      return length_unit_point;
    case Length::ETypes::PERCENT:
      return length_unit_percent;
    case Length::ETypes::FIT_CONTENT:
      // todo
      ASSERT(false);
      return length_unit_percent;
    default:
      return length_unit_point;
  }
}

} // namespace

namespace
{
using Views = std::vector<std::shared_ptr<LayoutNode>>;

bool layoutNodeHasExactSameChildren(flexbox_node* node, Views subviews)
{
  if (node->child_count() != subviews.size())
  {
    return false;
  }

  for (std::size_t i = 0; i < subviews.size(); ++i)
  {
    if (node->get_child(i) != subviews[i]->autoLayout()->getFlexItem())
    {
      return false;
    }
  }

  return true;
}

bool layoutNodeHasExactSameChildren(grid_layout* node, Views subviews)
{
  if (node->get_child_count() != subviews.size())
  {
    return false;
  }

  for (std::size_t i = 0; i < subviews.size(); ++i)
  {
    if (node->get_child(i) != subviews[i]->autoLayout()->getGridItem())
    {
      return false;
    }
  }

  return true;
}

void attachNodesFromViewHierachy(std::shared_ptr<LayoutNode> view)
{
  const auto autoLayout = view->autoLayout();
  if (const auto containerNode = autoLayout->getOrCreateFlexContainer())
  {
    if (autoLayout->isLeaf())
    {
      DEBUG(
        "attachNodesFromViewHierachy, leaf node, %s, %s",
        view->id().c_str(),
        view->name().c_str());
      view->detachChildrenFromFlexNodeTree();
      autoLayout->configureFlexContainer();
    }
    else
    {
      std::vector<std::shared_ptr<LayoutNode>> subviewsToInclude;
      for (auto subview : view->children())
      {
        if (!subview->isVisible())
        {
          DEBUG(
            "attachNodesFromViewHierachy, skip invisible child, %s, %s",
            subview->id().c_str(),
            subview->name().c_str());
          continue;
        }

        if (subview->autoLayout()->isEnabled() && subview->autoLayout()->isIncludedInLayout())
        {
          subviewsToInclude.push_back(subview);
        }
      }

      if (!layoutNodeHasExactSameChildren(containerNode, subviewsToInclude))
      {
        view->detachChildrenFromFlexNodeTree();

        for (auto subview : subviewsToInclude)
        {
          auto index = containerNode->child_count();
          auto childAutoLayout = subview->autoLayout();

          DEBUG(
            "attachNodesFromViewHierachy, flex container[%p] add child[%p, %s, %s], index: %d",
            containerNode,
            childAutoLayout->getFlexItem(),
            subview->id().c_str(),
            subview->name().c_str(),
            index);

          containerNode->add_child(childAutoLayout->takeFlexItem(), index);
          childAutoLayout->setFlexNodeIndex(index);
        }
      }

      autoLayout->configureFlexContainer();

      for (auto subview : subviewsToInclude)
      {
        subview->autoLayout()->configureFlexItemMargin();
        attachNodesFromViewHierachy(subview);
      }
    }
  }
  else if (const auto container = autoLayout->getGridContainer())
  {
    if (autoLayout->isLeaf())
    {
      container->clear_child();
    }
    else
    {
      std::vector<std::shared_ptr<LayoutNode>> subviewsToInclude;
      for (auto subview : view->children())
      {
        if (subview->autoLayout()->isEnabled() && subview->autoLayout()->isIncludedInLayout())
        {
          subviewsToInclude.push_back(subview);
        }
      }

      if (!layoutNodeHasExactSameChildren(container, subviewsToInclude))
      {
        container->clear_child();
        for (std::size_t i = 0; i < subviewsToInclude.size(); ++i)
        {
          DEBUG("attachNodesFromViewHierachy, grid container add child");
          container->add_child(subviewsToInclude[i]->autoLayout()->getGridItem(), i);
        }
      }

      for (auto subview : subviewsToInclude)
      {
        attachNodesFromViewHierachy(subview);
      }
    }
  }
}

void applyLayoutToViewHierarchy(
  std::shared_ptr<LayoutNode> view,
  bool                        preserveOrigin,
  bool                        isContainer)
{
  if (!view->isVisible())
  {
    DEBUG(
      "applyLayoutToViewHierarchy, view, %s, %s, [%p], invisible, return",
      view->id().c_str(),
      view->name().c_str(),
      view.get());
    return;
  }

  auto autoLayout = view->autoLayout();
  if (!autoLayout->isIncludedInLayout())
  {
    DEBUG("applyLayoutToViewHierarchy, not included in layout, return");
    return;
  }

  auto node = isContainer ? autoLayout->getFlexContainer() : autoLayout->getFlexItem();
  if (node)
  {
    auto size =
      view->swapWidthAndHeightIfNeeded({ node->get_layout_width(), node->get_layout_height() });
    Layout::Rect frame = { { .x = node->get_layout_left(), .y = node->get_layout_top() }, size };
    if (preserveOrigin)
    {
      frame = view->calculateResizedFrame(frame.size);
    }

    DEBUG(
      "applyLayoutToViewHierarchy, view, %s, %s, [%p], x=%f, y=%f, width=%f, height=%f",
      view->id().c_str(),
      view->name().c_str(),
      view.get(),
      frame.origin.x,
      frame.origin.y,
      frame.size.width,
      frame.size.height);

    autoLayout->setFrame(frame);
    view->setFrame(frame, false, false, true);
  }
  else if (auto node = autoLayout->getGridContainer())
  {
    // grid container
    auto size =
      view->swapWidthAndHeightIfNeeded({ node->get_layout_width(), node->get_layout_height() });
    Layout::Rect frame = { {}, size };
    if (preserveOrigin)
    {
      frame = view->calculateResizedFrame(frame.size);
    }
    DEBUG(
      "applyLayoutToViewHierarchy, view[%p], x=%d, y=%d, width=%d, height=%d",
      view.get(),
      static_cast<int>(frame.origin.x),
      static_cast<int>(frame.origin.y),
      static_cast<int>(frame.size.width),
      static_cast<int>(frame.size.height));
    autoLayout->setFrame(frame);
    view->setFrame(frame, false, false, true);

    // grid items
    auto& maybeLibFrames = autoLayout->gridItemFrames();
    if (!maybeLibFrames.has_value())
    {
      WARN("applyLayoutToViewHierarchy, no grid item frames");
      return;
    }

    auto& libFrames = *maybeLibFrames;
    auto& subviews = view->children();
    for (std::size_t i = 0; i < subviews.size(); ++i)
    {
      auto& libFrame = libFrames[i];
      auto& subview = subviews[i];

      Layout::Rect frame = {
        { .x = TO_VGG_LAYOUT_SCALAR(libFrame[1]), .y = TO_VGG_LAYOUT_SCALAR(libFrame[0]) },
        { .width = TO_VGG_LAYOUT_SCALAR(libFrame[2]), .height = TO_VGG_LAYOUT_SCALAR(libFrame[3]) }
      };
      DEBUG(
        "applyLayoutToViewHierarchy, view[%p, %s], x=%d, y=%d, width=%d, height=%d",
        subview.get(),
        subview->id().c_str(),
        static_cast<int>(frame.origin.x),
        static_cast<int>(frame.origin.y),
        static_cast<int>(frame.size.width),
        static_cast<int>(frame.size.height));
      subview->autoLayout()->setFrame(frame);
      subview->setFrame(frame, false, false, true);
    }
  }

  if (!autoLayout->isLeaf() && autoLayout->isContainer())
  {
    for (auto subview : view->children())
    {
      applyLayoutToViewHierarchy(subview, false, false);
    }
  }
}

} // namespace

void AutoLayout::applyLayout(bool preservingOrigin)
{
  if (isLeaf())
  {
    return;
  }

  if (!isContainer())
  {
    return;
  }

  if (auto sharedView = view.lock())
  {
    calculateLayout(sharedView->frame().size);
    applyLayoutToViewHierarchy(sharedView, preservingOrigin, true);
  }
}

Size AutoLayout::calculateLayout(Size size)
{
  auto sharedView = view.lock();
  if (!sharedView)
  {
    return size;
  }

  DEBUG("AutoLayout::calculateLayout, view[%p, %s]", sharedView.get(), sharedView->id().c_str());
  attachNodesFromViewHierachy(sharedView);

  if (auto flexNode = getFlexNode())
  {
    flexNode->calc_layout();
    return sharedView->swapWidthAndHeightIfNeeded(
      { flexNode->get_layout_width(), flexNode->get_layout_height() });
  }
  else if (auto gridContainer = getGridContainer())
  {
    DEBUG("AutoLayout::calculateLayout, grid container calculate");
    m_gridItemFrames = gridContainer->calc_layout(size.height, size.width);
    return sharedView->swapWidthAndHeightIfNeeded(
      { gridContainer->get_layout_width(), gridContainer->get_layout_height() });
  }
  else
  {
    WARN("AutoLayout::calculateLayout, no layout node, return parameter size");
    return size;
  }
}

bool AutoLayout::isLeaf()
{
  if (isEnabled())
  {
    if (auto sharedView = view.lock())
    {
      for (auto& child : sharedView->children())
      {
        auto autoLayout = child->autoLayout();
        if (child->isVisible() && autoLayout->isEnabled() && autoLayout->isIncludedInLayout())
        {
          return false;
        }
      }
    }
  }

  return true;
}

flexbox_node* AutoLayout::createFlexContainer()
{
  auto node = createFlexNode();

  if (auto sharedView = view.lock())
  {
    DEBUG(
      "AutoLayout::createFlexContainer, view[%p, %s], container[%p]",
      sharedView.get(),
      sharedView->id().c_str(),
      node);
  }

  return node;
}

flexbox_node* AutoLayout::createFlexItem()
{
  auto node = createFlexNode();

  if (auto sharedView = view.lock())
  {
    DEBUG(
      "AutoLayout::createFlexItem, view[%p, %s], item[%p]",
      sharedView.get(),
      sharedView->id().c_str(),
      node);
  }

  return node;
}

flexbox_node* AutoLayout::createFlexNode()
{
  auto node = new flexbox_node;

  m_flexNode.reset(node);
  m_flexNodePtr = node;

  return node;
}

flexbox_node* AutoLayout::parentFlexContainer()
{
  auto sharedView = view.lock();
  if (!sharedView)
  {
    return nullptr;
  }

  auto container = sharedView->containerAutoLayout();
  if (!container)
  {
    return nullptr;
  }

  return container->getFlexContainer();
}

void AutoLayout::resetGridContainer()
{
  if (auto sharedView = view.lock())
  {
    VERBOSE(
      "AutoLayout::resetGridContainer, view[%p, %s], grid container[%p]",
      sharedView.get(),
      sharedView->id().c_str(),
      m_gridContainerPtr);
  }

  m_gridContainer.reset();
  m_gridContainerPtr = nullptr;
}

void AutoLayout::resetGridItem()
{
  if (auto sharedView = view.lock())
  {
    VERBOSE(
      "AutoLayout::resetGridItem, view[%p, %s], grid item[%p]",
      sharedView.get(),
      sharedView->id().c_str(),
      m_gridItem.get());
  }

  m_gridItem.reset();
}

void AutoLayout::setFlexNodeIndex(std::size_t index)
{
  ASSERT(m_flexNodeIndex == static_cast<std::size_t>(-1));
  m_flexNodeIndex = index;
}

void AutoLayout::resetFlexNode()
{
  m_flexNode.reset();
  m_flexNodePtr = nullptr;
  m_flexNodeIndex = -1;
}

void AutoLayout::takeFlexNodeFromTree()
{
  auto sharedView = view.lock();
  if (!sharedView)
  {
    return;
  }

  auto container = sharedView->autoLayoutContainer();
  if (!container)
  {
    return;
  }

  auto flexContainerNode = container->autoLayout()->getFlexContainer();
  if (!flexContainerNode)
  {
    return;
  }

  if (m_flexNodeIndex >= flexContainerNode->child_count())
  {
    return;
  }

  DEBUG("AutoLayout::takeFlexNodeFromTree, index: %zu", m_flexNodeIndex);
  // Note: Pay attention to the deletion order, the last index must be deleted first
  auto flexNode = flexContainerNode->remove_child(m_flexNodeIndex);
  m_flexNode = std::move(flexNode);
  m_flexNodePtr = m_flexNode.get();
  m_flexNodeIndex = -1;

  DEBUG(
    "AutoLayout::takeFlexNodeFromTree, flex node [%p], view[%s, %s]",
    m_flexNodePtr,
    sharedView->id().c_str(),
    sharedView->name().c_str());
}

void AutoLayout::configure()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  auto sharedView = view.lock();
  if (!sharedView)
  {
    return;
  }

  if (isFlexContainer())
  {
    DEBUG(
      "AutoLayout::configure, flex container, view[%p, %s]",
      sharedView.get(),
      sharedView->id().c_str());

    resetGridContainer();
  }
  else if (isGridContainer())
  {
    DEBUG(
      "AutoLayout::configure, grid container, view[%p, %s]",
      sharedView.get(),
      sharedView->id().c_str());

    configureGridContainer();
  }

  if (const auto detail = sharedRule->getFlexItemRule())
  {
    DEBUG(
      "AutoLayout::configure, flex item, view[%p, %s]",
      sharedView.get(),
      sharedView->id().c_str());

    configureFlexItem(detail);
    configureFlexNodeSize(getFlexItem());

    resetGridItem();
  }
  else if (const auto detail = sharedRule->getGridItemRule())
  {
    DEBUG(
      "AutoLayout::configure, grid item, view[%p, %s]",
      sharedView.get(),
      sharedView->id().c_str());
    configureGridItem(detail);
    configureGridItemSize();
  }

  if (!sharedRule->isFlexContainer() && !sharedRule->isFlexItem())
  {
    resetFlexNode();
  }
}

void AutoLayout::configureFlexContainer()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  if (const auto detail = sharedRule->getFlexContainerRule())
  {
    // do not configure emplty container
    if (!isEmptyContainer())
    {
      configureFlexContainer(detail);
    }
    configureFlexNodeSize(getFlexContainer(), true);
  }
}

void AutoLayout::configureFlexItemMargin()
{
  auto node = getFlexItem();
  ASSERT(node);
  if (!node)
  {
    return;
  }

  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  auto sharedView = view.lock();
  if (!sharedView)
  {
    return;
  }

  auto container = sharedView->containerAutoLayout();
  if (!container)
  {
    return;
  }

  bool shouldConfigureStart{ false };
  bool shouldConfigureEnd{ false };
  if (container->isSmartSpacingAndFlexSpaceBetweenAndNoWrap())
  {
    if (isOnlyChild())
    {
      shouldConfigureStart = true;
      shouldConfigureEnd = true;
    }
    else if (isFirstChild())
    {
      shouldConfigureEnd = true;
    }
    else if (isLastChild())
    {
      shouldConfigureStart = true;
    }
    else
    {
      shouldConfigureStart = true;
      shouldConfigureEnd = true;
    }
  }

  // set to auto or RESET to undefine
  auto isHorizontalDirection = container->isHorizontalDirection();
  {
    auto side = isHorizontalDirection ? padding_left : padding_top;
    auto margin = shouldConfigureStart ? unit_auto : unit_undefine;
    node->set_margin(side, margin);
  }
  {
    auto side = isHorizontalDirection ? padding_right : padding_bottom;
    auto margin = shouldConfigureEnd ? unit_auto : unit_undefine;
    node->set_margin(side, margin);
  }
}

void AutoLayout::configureFlexNodeSize(flexbox_node* node, bool forContainer)
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  if (sharedRule->aspectRatio.has_value())
  {
    // todo
  }
  auto sharedView = view.lock();
  if (!sharedView)
  {
    return;
  }

  {
    VERBOSE(
      "AutoLayout::configureFlexNodeSize, view[%s, %s, %p]",
      sharedView->id().c_str(),
      sharedView->name().c_str(),
      sharedView.get());
  }

  auto width = sharedRule->width.value;
  auto height = sharedRule->height.value;

  Layout::Size size{ width.value, height.value };
  auto         swapWidthAndHeight = sharedView->shouldSwapWidthAndHeight();
  const auto&  modelSize = sharedView->bounds().size;
  if (width.types == Rule::Length::ETypes::PX && height.types == Rule::Length::ETypes::PX)
  {
    size = sharedView->rotatedSize(size);
  }
  else if (swapWidthAndHeight)
  {
    DEBUG("AutoLayout::configureFlexNodeSize, swap width and height");
    if (width.types == Length::ETypes::PX) // height.types != Length::ETypes::PX
    {
      size.width = modelSize.height;
    }
    else if (height.types == Length::ETypes::PX) // width.types != Length::ETypes::PX
    {
      size.height = modelSize.width;
    }
  }

  if (forContainer && isFlexContainer())
  {
    if ((width.types == Length::ETypes::FIT_CONTENT) && shouldChangeContainerHugWidth())
    {
      width.types = Length::ETypes::PX;
      width.value = swapWidthAndHeight ? modelSize.height : modelSize.width;
      size.width = width.value;
      DEBUG(
        "AutoLayout::configureFlexNodeSize, empty container with hug width, set to px: %f",
        size.width);
    }

    if ((height.types == Length::ETypes::FIT_CONTENT) && shouldChangeContainerHugHeight())
    {
      height.types = Length::ETypes::PX;
      height.value = swapWidthAndHeight ? modelSize.width : modelSize.height;
      size.height = height.value;
      DEBUG(
        "AutoLayout::configureFlexNodeSize, empty container with hug height, set to px: %f",
        size.height);
    }
  }

  VERBOSE("AutoLayout::configureFlexNodeSize, width %f", size.width);
  node->set_width(toLibUnit(width.types), size.width);
  if (sharedRule->maxWidth.has_value())
  {
    VERBOSE("AutoLayout::configureFlexNodeSize, max width %f", sharedRule->maxWidth->value.value);
    node->set_max_width(
      toLibUnit(sharedRule->maxWidth->value.types),
      sharedRule->maxWidth->value.value);
  }
  if (sharedRule->minWidth.has_value())
  {
    VERBOSE("AutoLayout::configureFlexNodeSize, min width %f", sharedRule->minWidth->value.value);
    node->set_min_width(
      toLibUnit(sharedRule->minWidth->value.types),
      sharedRule->minWidth->value.value);
  }

  VERBOSE("AutoLayout::configureFlexNodeSize, height %f", size.height);
  node->set_height(toLibUnit(height.types), size.height);
  if (sharedRule->maxHeight.has_value())
  {
    VERBOSE("AutoLayout::configureFlexNodeSize, max height %f", sharedRule->maxHeight->value.value);
    node->set_max_height(
      toLibUnit(sharedRule->maxHeight->value.types),
      sharedRule->maxHeight->value.value);
  }
  if (sharedRule->minHeight.has_value())
  {
    VERBOSE("AutoLayout::configureFlexNodeSize, min height %f", sharedRule->minHeight->value.value);
    node->set_min_height(
      toLibUnit(sharedRule->minHeight->value.types),
      sharedRule->minHeight->value.value);
  }
}

void AutoLayout::configureFlexItemAlignSelf(flexbox_node* node)
{
  auto sharedView = view.lock();
  if (!sharedView)
  {
    return;
  }

  auto container = sharedView->containerAutoLayout();
  if (!container)
  {
    return;
  }

  if (
    (container->isHorizontalDirection() && container->isFitContentHeight() &&
     is100PercentHeight()) ||
    (container->isVerticalDirection() && container->isFitContentWidth() && is100PercentWidth()))
  {
    // todo, What if there is rotation?
    node->set_align_self(align_items_stretch);
  }
}
bool AutoLayout::isHorizontalDirection()
{
  return directionIs(FlexboxLayout::EDirection::HORIZONTAL);
}
bool AutoLayout::isVerticalDirection()
{
  return directionIs(FlexboxLayout::EDirection::VERTICAL);
}
bool AutoLayout::directionIs(Rule::FlexboxLayout::EDirection direction)
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  if (const auto detail = sharedRule->getFlexContainerRule())
  {
    return detail->direction == direction;
  }
  return false;
}

bool AutoLayout::is100PercentWidth()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  return sharedRule->width.value.is100Percent();
}
bool AutoLayout::is100PercentHeight()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  return sharedRule->height.value.is100Percent();
}

bool AutoLayout::isFitContentWidth()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  return sharedRule->width.value.types == Length::ETypes::FIT_CONTENT;
}
bool AutoLayout::isFitContentHeight()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  return sharedRule->height.value.types == Length::ETypes::FIT_CONTENT;
}

bool AutoLayout::isSmartSpacingAndFlexSpaceBetweenAndNoWrap()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  const auto detail = sharedRule->getFlexContainerRule();
  if (!detail)
  {
    return false;
  }
  if (!detail->smartSpacing)
  {
    return false;
  }

  return detail->justifyContent == FlexboxLayout::EJustifyContent::SPACE_BETWEEN &&
         detail->wrap == FlexboxLayout::EWrap::NO_WRAP;
}

bool AutoLayout::isEmptyContainer()
{
  if (!isFlexContainer())
  {
    return false;
  }

  auto node = getFlexContainer();
  if (!node)
  {
    return false;
  }

  return 0 == node->child_count();
}

bool AutoLayout::isOnlyChild()
{
  auto node = getFlexItem();
  ASSERT(node);
  if (!node)
  {
    return false;
  }

  auto containerNode = parentFlexContainer();
  if (!containerNode)
  {
    return false;
  }

  return 1 == containerNode->child_count();
}

bool AutoLayout::isFirstChild()
{
  auto node = getFlexItem();
  ASSERT(node);
  if (!node)
  {
    return false;
  }

  auto containerNode = parentFlexContainer();
  if (!containerNode)
  {
    return false;
  }

  return containerNode->get_child(0) == node;
}

bool AutoLayout::isLastChild()
{
  auto node = getFlexItem();
  ASSERT(node);
  if (!node)
  {
    return false;
  }

  auto containerNode = parentFlexContainer();
  if (!containerNode)
  {
    return false;
  }

  return containerNode->get_child(containerNode->child_count() - 1) == node;
}

void AutoLayout::configureGridItemSize()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  auto node = getGridItem();

  if (sharedRule->aspectRatio.has_value())
  {
    // todo
  }

  node->set_width(grid_item::t_length{ toLibLengthUnit(sharedRule->width.value.types),
                                       sharedRule->width.value.value });
  if (sharedRule->maxWidth.has_value())
  {
    node->set_max_width(grid_item::t_length{ toLibLengthUnit(sharedRule->maxWidth->value.types),
                                             sharedRule->maxWidth->value.value });
  }
  if (sharedRule->minWidth.has_value())
  {
    node->set_min_width(grid_item::t_length{ toLibLengthUnit(sharedRule->minWidth->value.types),
                                             sharedRule->minWidth->value.value });
  }

  node->set_height(grid_item::t_length{ toLibLengthUnit(sharedRule->height.value.types),
                                        sharedRule->height.value.value });
  if (sharedRule->maxHeight.has_value())
  {
    node->set_max_height(grid_item::t_length{ toLibLengthUnit(sharedRule->maxHeight->value.types),
                                              sharedRule->maxHeight->value.value });
  }
  if (sharedRule->minHeight.has_value())
  {
    node->set_min_height(grid_item::t_length{ toLibLengthUnit(sharedRule->minHeight->value.types),
                                              sharedRule->minHeight->value.value });
  }
}

void AutoLayout::configureFlexContainer(Rule::FlexboxLayout* layout)
{
  auto node = getFlexContainer();
  if (!node)
  {
    node = createFlexContainer();
  }

  node->set_direction(toLibDirecion(layout->direction));
  node->set_justify_content(toLibJustifyContent(layout->justifyContent));
  node->set_align_items(toLibAlignItem(layout->alignItems));
  node->set_align_content(toLibAlignContent(layout->alignContent));
  node->set_wrap(toLibWrap(layout->wrap));
  node->set_gap(gap_row, layout->rowGap);
  node->set_gap(gap_column, layout->columnGap);
  node->set_padding(padding_top, layout->padding.top);
  node->set_padding(padding_right, layout->padding.right);
  node->set_padding(padding_bottom, layout->padding.bottom);
  node->set_padding(padding_left, layout->padding.left);

  if (layout->smartSpacing)
  {
    auto isHorizontalDirection = layout->direction == FlexboxLayout::EDirection::HORIZONTAL;
    if (layout->justifyContent == FlexboxLayout::EJustifyContent::SPACE_BETWEEN)
    {
      node->set_gap(isHorizontalDirection ? gap_column : gap_row, 0);
    }
    if (layout->alignContent == EAlignStyle::SPACE_BETWEEN)
    {
      node->set_gap(isHorizontalDirection ? gap_row : gap_column, 0);
    }
  }

  // todo, hanlde position
}

void AutoLayout::configureFlexItem(Rule::FlexboxItem* layout)
{
  auto node = getFlexItem();
  if (!node)
  {
    node = createFlexItem();
  }

  node->set_position(toLibPosition(layout->position.value));
  node->set_grow(layout->flexBasis);
  node->set_shrink(layout->flexBasis);

  if (layout->top.has_value())
  {
    node->set_ltrb(ltrb_top, layout->top->value);
  }
  if (layout->right.has_value())
  {
    node->set_ltrb(ltrb_right, layout->right->value);
  }
  if (layout->bottom.has_value())
  {
    node->set_ltrb(ltrb_bottom, layout->bottom->value);
  }
  if (layout->left.has_value())
  {
    node->set_ltrb(ltrb_left, layout->left->value);
  }

  configureFlexItemAlignSelf(node);

  // todo, hanlde position
}

void AutoLayout::configureGridContainer()
{
  auto layout = gridLayout();
  if (!layout)
  {
    return;
  }

  std::optional<uint32_t> minRow;
  auto                    columnCount = layout->expandStrategy.columnCount;
  if (layout->expandStrategy.strategy == ExpandStrategy::EStrategy::FIX_COLUMN)
  {
    minRow = layout->expandStrategy.minRow;
  }
  else
  {
    auto sharedView = view.lock();
    if (!sharedView)
    {
      WARN("configureGridContainer, invaid view, return");
      return;
    }
    columnCount = grid_layout::calc_column_count(
      sharedView->frame().size.width,
      layout->columnWidth.widthValue);
  }

  auto node = new grid_layout(columnCount, minRow);
  m_gridContainer.reset(node);
  m_gridContainerPtr = node;

  node->set_column_width(
    toLibColumnWidthStrategy(layout->columnWidth.strategy),
    layout->columnWidth.widthValue);
  node->set_row_height(
    toLibRowHeightStrategy(layout->rowHeight.strategy),
    layout->rowHeight.fixedValue);
  node->set_base_height(layout->baseHeight);
  node->set_column_gap(layout->columnGap);
  node->set_row_gap(layout->rowGap);
  node->set_grid_auto_flow(static_cast<grid_auto_flow>(layout->gridAutoFlow));

  node->set_padding(
    layout->padding.top,
    layout->padding.right,
    layout->padding.bottom,
    layout->padding.left);

  node->set_horizontal_align(toLibAlign(layout->cellAlign));
}

void AutoLayout::configureGridItem(Rule::GridItem* layout)
{
  auto node = new grid_item();
  m_gridItem.reset(node);

  node->set_item_pos_strategy(toLibItemPosStrategy(layout->itemPos.strategy));
  node->set_column_id(layout->itemPos.columnId);
  node->set_row_id(layout->itemPos.rowId);

  node->set_row_span(layout->rowSpan);
  node->set_column_span(layout->columnSpan);

  // todo, hanlde position, top/right/bottom/left

  node->set_horizontal_align(toLibAlign(layout->rowAlign));
  node->set_vertical_align(toLibAlign(layout->columnAlign));
}

void AutoLayout::setFrame(Rect newFrame)
{
  if (newFrame == m_frame)
  {
    return;
  }

  m_frame = newFrame;
}

void AutoLayout::updateSizeRule()
{
  auto sharedView = view.lock();
  auto sharedRule = rule.lock();
  if (isEnabled() && sharedView && sharedRule)
  {
    bool configureSize{ false };
    auto newSize = sharedView->frame().size;
    if (newSize != m_frame.size)
    {
      if (sharedRule->width.value.types == Rule::Length::ETypes::PX)
      {
        DEBUG(
          "AutoLayout::updateSizeRule, width = %f, view[%s, %s, %p]",
          newSize.width,
          sharedView->id().c_str(),
          sharedView->name().c_str(),
          sharedView.get());

        sharedRule->width.value.value = newSize.width;
        configureSize = true;
      }
      else
      {
        DEBUG("AutoLayout::updateSizeRule: width type is not PX, do not update");
      }

      if (sharedRule->height.value.types == Rule::Length::ETypes::PX)
      {
        DEBUG(
          "AutoLayout::updateSizeRule, height = %f, view[%s, %s, %p]",
          newSize.height,
          sharedView->id().c_str(),
          sharedView->name().c_str(),
          sharedView.get());
        sharedRule->height.value.value = newSize.height;
        configureSize = true;
      }
      else
      {
        DEBUG("AutoLayout::updateSizeRule: height type is not PX, do not update");
      }

      if (configureSize)
      {
        if (auto node = getFlexContainer())
        {
          configureFlexNodeSize(node, true);
        }
        else if (auto node = getFlexItem())
        {
          configureFlexNodeSize(node);
        }
      }
    }
  }
}

std::shared_ptr<LayoutNode> AutoLayout::setNeedsLayout()
{
  auto sharedView = view.lock();
  auto sharedRule = rule.lock();
  if (!sharedRule || !sharedView)
  {
    return nullptr;
  }

  if (sharedRule->isFlexItem() || sharedRule->isGridItem())
  {
    if (auto container = sharedView->autoLayoutContainer())
    {
      return container->autoLayout()->setNeedsLayout();
    }
  }

  if (sharedRule->isFlexContainer() || sharedRule->isGridContainer())
  {
    sharedView->setNeedLayout();
    return sharedView;
  }

  return nullptr;
}

flexbox_node* AutoLayout::getOrCreateFlexContainer()
{
  if (!isFlexContainer())
  {
    return nullptr;
  }

  auto node = getFlexContainer();
  if (!node)
  {
    node = createFlexContainer();
  }

  return node;
}

bool AutoLayout::isFlexOrGridItem()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  return sharedRule->isFlexItem() || sharedRule->isGridItem();
}

bool AutoLayout::isIncludedInLayout()
{
  return !isAboslutePosition();
}

bool AutoLayout::isAboslutePosition()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  if (const auto detail = sharedRule->getFlexItemRule())
  {
    return detail->position.value == Position::ETypes::ABSOLUTE;
  }

  return false;
}

void AutoLayout::removeSubtree()
{
  if (auto node = getFlexContainer())
  {
    while (node->child_count() > 0)
    {
      node->remove_child(node->child_count() - 1);
    }
  }
}

bool AutoLayout::isContainer()
{
  return isFlexContainer() || isGridContainer();
}

bool AutoLayout::isFlexContainer()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return false;
  }

  return sharedRule->getFlexContainerRule();
}

bool AutoLayout::isGridContainer()
{
  return gridLayout() != nullptr;
}

GridLayout* AutoLayout::gridLayout()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return nullptr;
  }

  return sharedRule->getGridContainerRule();
}

void AutoLayout::isFixedSize(bool& outWidth, bool& outHeight)
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
    return;

  const auto& widthValue = sharedRule->width.value;
  const auto& heightValue = sharedRule->height.value;
  outWidth = widthValue.types == Rule::Length::ETypes::PX;
  outHeight = heightValue.types == Rule::Length::ETypes::PX;
  isAsFixedSize(outWidth, outHeight);
}

void AutoLayout::isAsFixedSize(bool& inOutWidth, bool& inOutHeight)
{
  if (inOutWidth && inOutHeight)
    return;

  if (!isFlexContainer())
    return;

  const auto& sharedRule = rule.lock();
  if (!sharedRule)
    return;

  if (!inOutWidth)
  {
    const auto& width = sharedRule->width.value;
    inOutWidth = (width.types == Length::ETypes::FIT_CONTENT) && shouldChangeContainerHugWidth();
  }

  if (!inOutHeight)
  {
    const auto& height = sharedRule->height.value;
    inOutHeight = (height.types == Length::ETypes::FIT_CONTENT) && shouldChangeContainerHugHeight();
  }
}

bool AutoLayout::shouldChangeContainerHugWidth()
{
  if (isEmptyContainer())
    return true;

  if (isHorizontalDirection())
    return m_hasUnkownWidthChild;
  else
    return !m_hasFixedWidthChild; // has no fixed width child, cannot calculate hug width
}

bool AutoLayout::shouldChangeContainerHugHeight()
{
  if (isEmptyContainer())
    return true;

  if (isVerticalDirection())
    return m_hasUnknownHeightChild;
  else
    return !m_hasFixedHeightChild;
}

} // namespace Internal
} // namespace Layout
} // namespace VGG
