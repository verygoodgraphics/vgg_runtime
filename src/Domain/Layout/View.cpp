#include "View.hpp"

#include "AutoLayout.hpp"
#include "Log.h"
#include "Rule.hpp"

using namespace VGG::Layout::Internal::Rule;

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

namespace VGG
{

std::shared_ptr<Layout::Internal::AutoLayout> LayoutView::autoLayout() const
{
  return m_autoLayout;
}

std::shared_ptr<Layout::Internal::AutoLayout> LayoutView::createAutoLayout()
{
  m_autoLayout.reset(new Layout::Internal::AutoLayout);
  m_autoLayout->view = weak_from_this();

  return m_autoLayout;
}

std::shared_ptr<LayoutView> LayoutView::autoLayoutContainer()
{
  return m_parent.lock();
}

std::shared_ptr<flexbox_node> LayoutView::createFlexboxNode()
{
  flexbox_node::p_node node{ new flexbox_node };
  m_autoLayout->flexNode = node;

  return m_autoLayout->flexNode;
}

std::shared_ptr<grid_layout> LayoutView::createGridNode()
{
  // todo
  return m_autoLayout->gridNode;
}

void LayoutView::applyLayout()
{
  for (auto subview : m_children)
  {
    subview->applyLayout();
  }

  if (m_autoLayout)
  {
    m_autoLayout->applyLayout(false);
  }
}

void LayoutView::setNeedLayout()
{
  m_needsLayout = true;
}

void LayoutView::layoutIfNeeded()
{
  for (auto subview : m_children)
  {
    subview->layoutIfNeeded();
  }

  if (m_needsLayout)
  {
    m_needsLayout = false;

    configureAutoLayout();
    applyLayout();
  }
}

const Layout::Rect& LayoutView::frame() const
{
  return m_frame;
}
void LayoutView::setFrame(const Layout::Rect& frame)
{
  m_frame = frame;

  // todo, scale subview if no layout
  // todo, update json model
  // todo, mark dirty
  m_autoLayout->frameChanged();
}

void LayoutView::configureAutoLayout()
{
  if (!m_autoLayout)
  {
    return;
  }
  auto rule = m_autoLayout->rule;
  if (!rule)
  {
    return;
  }

  if (const auto detail = std::get_if<FlexboxLayout>(&rule->layout))
  {
    DEBUG("LayoutView::configureAutoLayout, flex layout, view[%p, %s]", this, path().c_str());

    auto node = createFlexboxNode();
    configureNode(node, rule);

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
  else if (const auto detail = std::get_if<GridLayout>(&rule->layout))
  {
    DEBUG("LayoutView::configureAutoLayout, grid layout, view[%p, %s]", this, path().c_str());

    // todo
    // auto node = new grid_layout;
    // autoLayout->gridNode.reset(node);
  }

  if (const auto detail = std::get_if<FlexboxItem>(&rule->item_in_layout))
  {
    DEBUG("LayoutView::configureAutoLayout, flex item , view[%p, %s]", this, path().c_str());

    auto node = autoLayout()->flexNode;
    if (!node)
    {
      node = createFlexboxNode();
    }
    configureNode(node, rule);

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
  else if (const auto detail = std::get_if<GridItem>(&rule->item_in_layout))
  {
    DEBUG("LayoutView::configureAutoLayout, grid item , view[%p, %s]", this, path().c_str());
    // todo
  }
}

void LayoutView::configureNode(std::shared_ptr<flexbox_node> node,
                               std::shared_ptr<VGG::Layout::Internal::Rule::Rule> rule)
{
  node->set_width(toLibUnit(rule->width.value.types), rule->width.value.value);
  if (rule->max_width.has_value())
  {
    node->set_max_width(toLibUnit(rule->max_width->value.types), rule->max_width->value.value);
  }
  if (rule->min_width.has_value())
  {
    node->set_min_width(toLibUnit(rule->min_width->value.types), rule->min_width->value.value);
  }

  node->set_height(toLibUnit(rule->height.value.types), rule->height.value.value);
  if (rule->max_height.has_value())
  {
    node->set_max_height(toLibUnit(rule->max_height->value.types), rule->max_height->value.value);
  }
  if (rule->min_height.has_value())
  {
    node->set_min_height(toLibUnit(rule->min_height->value.types), rule->min_height->value.value);
  }

  if (rule->aspect_ratio.has_value())
  {
    // todo
  }
}
} // namespace VGG