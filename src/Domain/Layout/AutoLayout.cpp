#include "AutoLayout.hpp"

#include "Utility/Log.hpp"
#include "Rule.hpp"
#include "Node.hpp"

#include <memory>
#include <vector>

#undef DEBUG
#define DEBUG(msg, ...)

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
  }
}

} // namespace

namespace
{
using Views = std::vector<std::shared_ptr<LayoutNode>>;

void removeAllChildren(flexbox_node* node)
{
  DEBUG("removeAllChildren, flex node[%p]", node);
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

  for (auto i = 0; i < subviews.size(); ++i)
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
  if (const auto node = autoLayout->getFlexContainer())
  {
    if (autoLayout->isLeaf())
    {
      removeAllChildren(node);
    }
    else
    {
      std::vector<std::shared_ptr<LayoutNode>> subviewsToInclude;
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
          DEBUG("attachNodesFromViewHierachy, flex container[%p] add child[%s]",
                node,
                subviewsToInclude[i]->path().c_str());
          node->add_child(subviewsToInclude[i]->autoLayout()->takeFlexItem(), i);
        }
      }

      for (auto subview : subviewsToInclude)
      {
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
        if (subview->autoLayout()->isEnabled() && subview->autoLayout()->isIncludedInLayout)
        {
          subviewsToInclude.push_back(subview);
        }
      }

      if (!layoutNodeHasExactSameChildren(container, subviewsToInclude))
      {
        container->clear_child();
        for (auto i = 0; i < subviewsToInclude.size(); ++i)
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

void applyLayoutToViewHierarchy(std::shared_ptr<LayoutNode> view,
                                bool preserveOrigin,
                                bool container)
{
  auto autoLayout = view->autoLayout();
  if (!autoLayout->isIncludedInLayout)
  {
    return;
  }

  auto node = container ? autoLayout->getFlexContainer() : autoLayout->getFlexItem();
  if (node)
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
  else if (auto gridContainer = autoLayout->getGridContainer())
  {
    // grid container
    Point origin = view->frame().origin;
    Layout::Rect frame = { origin,
                           { .width = TO_VGG_LAYOUT_SCALAR(gridContainer->get_layout_width()),
                             .height = TO_VGG_LAYOUT_SCALAR(gridContainer->get_layout_height()) } };
    DEBUG("applyLayoutToViewHierarchy, view[%p, %s], x=%d, y=%d, width=%d, height=%d",
          view.get(),
          view->path().c_str(),
          static_cast<int>(frame.origin.x),
          static_cast<int>(frame.origin.y),
          static_cast<int>(frame.size.width),
          static_cast<int>(frame.size.height));
    autoLayout->frame = frame;
    view->setFrame(frame);

    // grid items
    auto& maybeLibFrames = autoLayout->gridItemFrames();
    if (!maybeLibFrames.has_value())
    {
      WARN("applyLayoutToViewHierarchy, no grid item frames");
      return;
    }

    auto& libFrames = *maybeLibFrames;
    auto& subviews = view->children();
    for (auto i = 0; i < subviews.size(); ++i)
    {
      auto& libFrame = libFrames[i];
      auto& subview = subviews[i];

      Layout::Rect frame = {
        { .x = TO_VGG_LAYOUT_SCALAR(libFrame[1]), .y = TO_VGG_LAYOUT_SCALAR(libFrame[0]) },
        { .width = TO_VGG_LAYOUT_SCALAR(libFrame[2]), .height = TO_VGG_LAYOUT_SCALAR(libFrame[3]) }
      };
      DEBUG("applyLayoutToViewHierarchy, view[%p, %s], x=%d, y=%d, width=%d, height=%d",
            subview.get(),
            subview->path().c_str(),
            static_cast<int>(frame.origin.x),
            static_cast<int>(frame.origin.y),
            static_cast<int>(frame.size.width),
            static_cast<int>(frame.size.height));
      subview->autoLayout()->frame = frame;
      subview->setFrame(frame);
    }
  }

  if (!autoLayout->isLeaf())
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

  if (auto sharedView = view.lock())
  {
    calculateLayout(sharedView->frame().size);
    applyLayoutToViewHierarchy(sharedView, preservingOrigin, true);
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
  else if (auto gridContainer = getGridContainer())
  {
    DEBUG("AutoLayout::calculateLayout, grid container calculate");
    m_gridItemFrames = gridContainer->calc_layout(size.height, size.width);
    return { TO_VGG_LAYOUT_SCALAR(gridContainer->get_layout_width()),
             TO_VGG_LAYOUT_SCALAR(gridContainer->get_layout_height()) };
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
      // todo, old type is PERCENT
      sharedRule->width.value.types = Rule::Length::ETypes::PX;
      sharedRule->width.value.value = newSize.width;

      sharedRule->height.value.types = Rule::Length::ETypes::PX;
      sharedRule->height.value.value = newSize.height;

      if (sharedRule->isFlexContainer() || sharedRule->isGridContainer())
      {
        sharedView->setNeedLayout();
      }

      if (sharedRule->isFlexItem() || sharedRule->isGridItem())
      {
        if (auto container = sharedView->autoLayoutContainer())
        {
          container->setNeedLayout();
        }
      }
    }
  }
}

flexbox_node* AutoLayout::createFlexContainer()
{
  auto node = createFlexNode();

  if (auto sharedView = view.lock())
  {
    DEBUG("AutoLayout::createFlexContainer, view[%p, %s], container[%p]",
          sharedView.get(),
          sharedView->path().c_str(),
          node);
  }

  return node;
}

flexbox_node* AutoLayout::createFlexItem()
{
  auto node = createFlexNode();

  if (auto sharedView = view.lock())
  {
    DEBUG("AutoLayout::createFlexItem, view[%p, %s], item[%p]",
          sharedView.get(),
          sharedView->path().c_str(),
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

void AutoLayout::resetGridContainer()
{
  if (auto sharedView = view.lock())
  {
    DEBUG("AutoLayout::resetGridContainer, view[%p, %s], grid container[%p]",
          sharedView.get(),
          sharedView->path().c_str(),
          m_gridContainerPtr);
  }

  m_gridContainer.reset();
  m_gridContainerPtr = nullptr;
}

void AutoLayout::resetGridItem()
{
  if (auto sharedView = view.lock())
  {
    DEBUG("AutoLayout::resetGridItem, view[%p, %s], grid item[%p]",
          sharedView.get(),
          sharedView->path().c_str(),
          m_gridItem.get());
  }

  m_gridItem.reset();
}

void AutoLayout::resetFlexNode()
{
  m_flexNode.reset();
  m_flexNodePtr = nullptr;
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

  if (const auto detail = sharedRule->getFlexContainerRule())
  {
    DEBUG("AutoLayout::configure, flex container, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());

    m_isContainer = true;
    configureFlexContainer(detail);
    configureFlexNodeSize(getFlexContainer());

    resetGridContainer();
  }
  else if (const auto detail = sharedRule->getGridContainerRule())
  {
    DEBUG("AutoLayout::configure, grid container, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());

    m_isContainer = true;
    configureGridContainer(detail);
  }
  else
  {
    m_isContainer = false;
  }

  if (const auto detail = sharedRule->getFlexItemRule())
  {
    DEBUG("AutoLayout::configure, flex item, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());

    configureFlexItem(detail);
    configureFlexNodeSize(getFlexItem());

    resetGridItem();
  }
  else if (const auto detail = sharedRule->getGridItemRule())
  {
    DEBUG("AutoLayout::configure, grid item, view[%p, %s]",
          sharedView.get(),
          sharedView->path().c_str());
    configureGridItem(detail);
    configureGridItemSize();
  }

  if (!sharedRule->isFlexContainer() && !sharedRule->isFlexItem())
  {
    resetFlexNode();
  }
}

void AutoLayout::configureFlexNodeSize(flexbox_node* node)
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  if (sharedRule->aspect_ratio.has_value())
  {
    // todo
  }

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
}

void AutoLayout::configureGridItemSize()
{
  auto sharedRule = rule.lock();
  if (!sharedRule)
  {
    return;
  }

  auto node = getGridItem();

  if (sharedRule->aspect_ratio.has_value())
  {
    // todo
  }

  node->set_width(grid_item::t_length{ toLibLengthUnit(sharedRule->width.value.types),
                                       sharedRule->width.value.value });
  if (sharedRule->max_width.has_value())
  {
    node->set_max_width(grid_item::t_length{ toLibLengthUnit(sharedRule->max_width->value.types),
                                             sharedRule->max_width->value.value });
  }
  if (sharedRule->min_width.has_value())
  {
    node->set_min_width(grid_item::t_length{ toLibLengthUnit(sharedRule->min_width->value.types),
                                             sharedRule->min_width->value.value });
  }

  node->set_height(grid_item::t_length{ toLibLengthUnit(sharedRule->height.value.types),
                                        sharedRule->height.value.value });
  if (sharedRule->max_height.has_value())
  {
    node->set_max_height(grid_item::t_length{ toLibLengthUnit(sharedRule->max_height->value.types),
                                              sharedRule->max_height->value.value });
  }
  if (sharedRule->min_height.has_value())
  {
    node->set_min_height(grid_item::t_length{ toLibLengthUnit(sharedRule->min_height->value.types),
                                              sharedRule->min_height->value.value });
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
  node->set_justify_content(toLibJustifyContent(layout->justify_content));
  node->set_align_items(toLibAlignItem(layout->align_items));
  node->set_align_content(toLibAlignContent(layout->align_content));
  node->set_wrap(toLibWrap(layout->wrap));
  node->set_gap(gap_row, layout->row_gap);
  node->set_gap(gap_column, layout->column_gap);
  node->set_padding(padding_top, layout->padding.top);
  node->set_padding(padding_right, layout->padding.right);
  node->set_padding(padding_bottom, layout->padding.bottom);
  node->set_padding(padding_left, layout->padding.left);

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
  node->set_grow(layout->flex_grow);

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

  // todo, hanlde position
}

void AutoLayout::configureGridContainer(GridLayout* layout)
{
  std::optional<uint32_t> minRow;
  auto columnCount = layout->expand_strategy.column_count;
  if (layout->expand_strategy.strategy == ExpandStrategy::EStrategy::FIX_COLUMN)
  {
    minRow = layout->expand_strategy.min_row;
  }
  else
  {
    auto sharedView = view.lock();
    if (!sharedView)
    {
      WARN("configureGridContainer, invaid view, return");
      return;
    }
    columnCount = grid_layout::calc_column_count(sharedView->frame().size.width,
                                                 layout->column_width.width_value);
  }

  auto node = new grid_layout(columnCount, minRow);
  m_gridContainer.reset(node);
  m_gridContainerPtr = node;

  node->set_column_width(toLibColumnWidthStrategy(layout->column_width.strategy),
                         layout->column_width.width_value);
  node->set_row_height(toLibRowHeightStrategy(layout->row_height.strategy),
                       layout->row_height.fixed_value);
  node->set_base_height(layout->base_height);
  node->set_column_gap(layout->column_gap);
  node->set_row_gap(layout->row_gap);
  node->set_grid_auto_flow(static_cast<grid_auto_flow>(layout->grid_auto_flow));

  node->set_padding(layout->padding.top,
                    layout->padding.right,
                    layout->padding.bottom,
                    layout->padding.left);

  node->set_horizontal_align(toLibAlign(layout->cell_align));
}

void AutoLayout::configureGridItem(Rule::GridItem* layout)
{
  auto node = new grid_item();
  m_gridItem.reset(node);

  node->set_item_pos_strategy(toLibItemPosStrategy(layout->item_pos.strategy));
  node->set_column_id(layout->item_pos.column_id);
  node->set_row_id(layout->item_pos.row_id);

  node->set_row_span(layout->row_span);
  node->set_column_span(layout->column_span);

  // todo, hanlde position, top/right/bottom/left

  node->set_horizontal_align(toLibAlign(layout->row_align));
  node->set_vertical_align(toLibAlign(layout->column_align));
}

} // namespace Internal
} // namespace Layout
} // namespace VGG
