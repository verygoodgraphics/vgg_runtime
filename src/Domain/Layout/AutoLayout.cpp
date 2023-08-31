#include "AutoLayout.hpp"

#include "Log.h"
#include "Rule.hpp"
#include "View.hpp"

#include <memory>
#include <vector>

using namespace VGG::Layout::Internal::Rule;

namespace VGG
{
class LayoutView;

namespace Layout
{

namespace Internal
{

namespace
{
using LengthTypes = VGG::Layout::Internal::Rule::Length::Types;
unit toLibUnit(LengthTypes type)
{
  switch (type)
  {
    case LengthTypes::px:
      return unit_point;
    case LengthTypes::percent:
      return unit_percent;
    case LengthTypes::fit_content:
      return unit_auto;
    default:
      return unit_undefine;
  }
}

position toLibPosition(Position::Types type)
{
  switch (type)
  {
    case Position::Types::Relative:
      return position_relative;
    case Position::Types::Absolute:
      return position_absolute;

    default: // todo
      return position_absolute;
  }
}

direction toLibDirecion(FlexboxLayout::Direction type)
{
  switch (type)
  {
    case FlexboxLayout::Direction::Horizontal:
      return direction_row;

    default:
      return direction_column;
  }
}

justify_content toLibJustifyContent(FlexboxLayout::JustifyContent type)
{
  switch (type)
  {
    case FlexboxLayout::JustifyContent::Start:
      return justify_content_flex_start;
    case FlexboxLayout::JustifyContent::Center:
      return justify_content_center;
    case FlexboxLayout::JustifyContent::End:
      return justify_content_flex_end;
    case FlexboxLayout::JustifyContent::SpaceBetween:
      return justify_content_space_between;
    case FlexboxLayout::JustifyContent::SpaceAround:
      return justify_content_space_around;
    case FlexboxLayout::JustifyContent::SpaceEvenly:
      return justify_content_space_evenly;
  }
}

align_content toLibAlignContent(AlignStyle type)
{
  switch (type)
  {
    case AlignStyle::Start:
      return align_content_flex_start;
    case AlignStyle::Center:
      return align_content_center;
    case AlignStyle::End:
      return align_content_flex_end;
  }
}

align_items toLibAlignItem(AlignStyle type)
{
  switch (type)
  {
    case AlignStyle::Start:
      return align_items_flex_start;
    case AlignStyle::Center:
      return align_items_center;
    case AlignStyle::End:
      return align_items_flex_end;
  }
}

wrap toLibWrap(FlexboxLayout::Wrap type)
{
  switch (type)
  {
    case FlexboxLayout::Wrap::NoWrap:
      return wrap_no_wrap;
    case FlexboxLayout::Wrap::Wrap:
      return wrap_wrap;
  }
}

} // namespace

namespace
{
using Views = std::vector<std::shared_ptr<LayoutView>>;

void removeAllChildren(flexbox_node* node)
{
  while (node->child_count() > 0)
  {
    node->remove_child(node->child_count() - 1);
  }
}

bool layoutNodeHasExactSameChildren(flexbox_node* node, Views subviews)
{
  if (node->child_count() != subviews.size())
  {
    return false;
  }

  for (auto i = 0; i < subviews.size(); ++i)
  {
    if (node->get_child(i) != subviews[i]->autoLayout()->getFlexNode())
    {
      return false;
    }
  }

  return true;
}

void attachNodesFromViewHierachy(std::shared_ptr<LayoutView> view)
{
  // todo
  const auto autoLayout = view->autoLayout();
  if (const auto node = autoLayout->getFlexNode())
  {
    if (autoLayout->isLeaf())
    {
      removeAllChildren(node);
    }
    else
    {
      std::vector<std::shared_ptr<LayoutView>> subviewsToInclude;
      for (auto subview : view->children())
      {
        if (subview->autoLayout()->isEnabled() && subview->autoLayout()->isIncludedInLayout)
        {
          subviewsToInclude.push_back(subview);
        }
      }

      if (!layoutNodeHasExactSameChildren(node, subviewsToInclude))
      {
        removeAllChildren(node);
        for (auto i = 0; i < subviewsToInclude.size(); ++i)
        {
          DEBUG("attachNodesFromViewHierachy, flex node add child");
          node->add_child(subviewsToInclude[i]->autoLayout()->takeFlexNode(), i);
        }
      }

      for (auto subview : subviewsToInclude)
      {
        attachNodesFromViewHierachy(subview);
      }
    }
  }
}

void applyLayoutToViewHierarchy(std::shared_ptr<LayoutView> view, bool preserveOrigin)
{
  auto autoLayout = view->autoLayout();
  if (!autoLayout->isIncludedInLayout)
  {
    return;
  }

  if (auto node = autoLayout->getFlexNode())
  {
    Point origin;
    if (preserveOrigin)
    {
      origin = view->frame().origin;
    }
    Layout::Rect frame = {
      { .x = node->get_layout_left() + origin.x, .y = node->get_layout_top() + origin.y },
      { .width = node->get_layout_width(), .height = node->get_layout_height() }
    };

    DEBUG("applyLayoutToViewHierarchy, view[%p, %s], x=%d, y=%d, width=%d, height=%d",
          view.get(),
          view->path().c_str(),
          static_cast<int>(frame.origin.x),
          static_cast<int>(frame.origin.y),
          static_cast<int>(frame.size.width),
          static_cast<int>(frame.size.height));

    autoLayout->frame = frame;
    view->setFrame(frame);
  }

  if (!autoLayout->isLeaf())
  {
    for (auto subview : view->children())
    {
      applyLayoutToViewHierarchy(subview, false);
    }
  }

  // todo, gridNode
}

} // namespace

void AutoLayout::applyLayout(bool preservingOrigin)
{
  if (isLeaf())
  {
    return;
  }

  if (auto sharedView = view.lock())
  {
    calculateLayout(sharedView->frame().size);
    applyLayoutToViewHierarchy(sharedView, preservingOrigin);
  }
}

Size AutoLayout::calculateLayout(Size size)
{
  if (auto sharedView = view.lock())
  {
    DEBUG("AutoLayout::calculateLayout, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());
    attachNodesFromViewHierachy(sharedView);
  }

  if (auto flexNode = getFlexNode())
  {
    flexNode->calc_layout();
    return { flexNode->get_layout_width(), flexNode->get_layout_height() };
  }
  else if (auto gridNode = getGridNode())
  {
    DEBUG("AutoLayout::calculateLayout, grid node calculate");
    gridNode->calc_layout();
    return { TO_VGG_LAYOUT_SCALAR(gridNode->get_layout_width()),
             TO_VGG_LAYOUT_SCALAR(gridNode->get_layout_height()) };
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
        if (autoLayout->isEnabled() && autoLayout->isIncludedInLayout)
        {
          return false;
        }
      }
    }
  }

  return true;
}

void AutoLayout::frameChanged()
{
  auto sharedView = view.lock();
  auto sharedRule = rule.lock();
  if (isEnabled() && sharedView && sharedRule)
  {
    auto newSize = sharedView->frame().size;
    if (newSize != frame.size)
    {
      // todo, old type is percent
      sharedRule->width.value.types = Rule::Length::Types::px;
      sharedRule->width.value.value = newSize.width;

      sharedRule->height.value.types = Rule::Length::Types::px;
      sharedRule->height.value.value = newSize.height;

      if (sharedRule->isFlexConatiner() || sharedRule->isFlexConatiner())
      {
        sharedView->setNeedLayout();
      }
      else if (sharedRule->isFlexItem() || sharedRule->isGridItem())
      {
        if (auto container = sharedView->autoLayoutContainer())
        {
          container->setNeedLayout();
        }

        // todo, scale descendant node
      }
    }
  }
}

std::unique_ptr<flexbox_node>& AutoLayout::takeFlexNode()
{
  return m_flexNode;
}

std::unique_ptr<grid_layout>& AutoLayout::takeGridNode()
{
  return m_gridNode;
}

flexbox_node* AutoLayout::createFlexNode()
{
  m_flexNodePtr = new flexbox_node;
  m_flexNode.reset(m_flexNodePtr);
  return m_flexNode.get();
}

grid_layout* AutoLayout::createGridNode()
{
  // todo
  return nullptr;
}

flexbox_node* AutoLayout::getFlexNode()
{
  return m_flexNodePtr;
}

grid_layout* AutoLayout::getGridNode()
{
  return m_gridNodePtr;
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

  if (const auto detail = std::get_if<FlexboxLayout>(&sharedRule->layout))
  {
    DEBUG("LayoutView::configureAutoLayout, flex layout, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());

    auto node = createFlexNode();
    configureNode(node, sharedRule);

    node->set_direction(toLibDirecion(detail->direction));
    node->set_justify_content(toLibJustifyContent(detail->justify_content));
    node->set_align_items(toLibAlignItem(detail->align_items));
    node->set_align_content(toLibAlignContent(detail->align_content));
    node->set_wrap(toLibWrap(detail->wrap));
    node->set_gap(gap_row, detail->row_gap);
    node->set_gap(gap_column, detail->column_gap);
    node->set_padding(padding_top, detail->padding.top);
    node->set_padding(padding_right, detail->padding.right);
    node->set_padding(padding_bottom, detail->padding.bottom);
    node->set_padding(padding_left, detail->padding.left);
  }
  else if (const auto detail = std::get_if<GridLayout>(&sharedRule->layout))
  {
    DEBUG("LayoutView::configureAutoLayout, grid layout, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());

    // todo
    // auto node = new grid_layout;
    // autoLayout->gridNode.reset(node);
  }

  if (const auto detail = std::get_if<FlexboxItem>(&sharedRule->item_in_layout))
  {
    DEBUG("LayoutView::configureAutoLayout, flex item, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());

    auto node = getFlexNode();
    if (!node)
    {
      node = createFlexNode();
    }
    configureNode(node, sharedRule);

    node->set_position(toLibPosition(detail->position.value));
    node->set_grow(detail->flex_grow);

    if (detail->top.has_value())
    {
      node->set_ltrb(ltrb_top, detail->top->value);
    }
    if (detail->right.has_value())
    {
      node->set_ltrb(ltrb_right, detail->right->value);
    }
    if (detail->bottom.has_value())
    {
      node->set_ltrb(ltrb_bottom, detail->bottom->value);
    }
    if (detail->left.has_value())
    {
      node->set_ltrb(ltrb_left, detail->left->value);
    }
  }
  else if (const auto detail = std::get_if<GridItem>(&sharedRule->item_in_layout))
  {
    DEBUG("LayoutView::configureAutoLayout, grid item, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());
    // todo
  }
}

void AutoLayout::configureNode(flexbox_node* node,
                               std::shared_ptr<VGG::Layout::Internal::Rule::Rule> sharedRule)
{
  node->set_width(toLibUnit(sharedRule->width.value.types), sharedRule->width.value.value);
  if (sharedRule->max_width.has_value())
  {
    node->set_max_width(toLibUnit(sharedRule->max_width->value.types),
                        sharedRule->max_width->value.value);
  }
  if (sharedRule->min_width.has_value())
  {
    node->set_min_width(toLibUnit(sharedRule->min_width->value.types),
                        sharedRule->min_width->value.value);
  }

  node->set_height(toLibUnit(sharedRule->height.value.types), sharedRule->height.value.value);
  if (sharedRule->max_height.has_value())
  {
    node->set_max_height(toLibUnit(sharedRule->max_height->value.types),
                         sharedRule->max_height->value.value);
  }
  if (sharedRule->min_height.has_value())
  {
    node->set_min_height(toLibUnit(sharedRule->min_height->value.types),
                         sharedRule->min_height->value.value);
  }

  if (sharedRule->aspect_ratio.has_value())
  {
    // todo
  }
}

} // namespace Internal
} // namespace Layout
} // namespace VGG