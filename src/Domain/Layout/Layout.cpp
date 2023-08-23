#include "Layout.hpp"

#include "Bridge.hpp"
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Log.h"
#include "Rule.hpp"

using namespace VGG;
using namespace VGG::Layout::Internal::Rule;

constexpr auto FLIP_Y_FACTOR = -1;

namespace
{
using LengthTypes = Layout::Internal::Rule::Length::Types;
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

void Layout::Layout::layout(Size size)
{
  if (size != m_size)
  {
    // todo
    m_size = size;
  }
}

std::shared_ptr<LayoutView> Layout::Layout::layoutTree()
{
  if (!m_designJson.is_object())
  {
    WARN("invalid design file");
    return {};
  }

  m_layout_tree.reset(new LayoutView{ "/", {} });

  json::json_pointer frames_path{ "/frames" };
  for (auto i = 0; i < m_designJson[K_FRAMES].size(); ++i)
  {
    auto path = frames_path / i;
    createOneLayoutView(m_designJson[path], path, m_layout_tree);
  }

  return m_layout_tree;
}

std::shared_ptr<LayoutView> Layout::Layout::createOneLayoutView(const nlohmann::json& j,
                                                                json::json_pointer currentPath,
                                                                std::shared_ptr<LayoutView> parent)
{
  if (!j.is_object())
  {
    WARN("create one layout view from json object, json is not object, return");
    return nullptr;
  }

  if (!isLayoutNode(j))
  {
    return nullptr;
  }

  auto frame = j[K_FRAME].get<Rect>();
  frame.origin.y *= FLIP_Y_FACTOR;

  auto layoutView = std::make_shared<LayoutView>(currentPath.to_string(), frame);
  if (parent)
  {
    parent->addChild(layoutView);
  }

  for (auto& [key, val] : j.items())
  {
    auto path = currentPath;
    path /= key;

    createOneOrMoreLayoutViews(val, path, layoutView);
  }

  return layoutView;
}

void Layout::Layout::createLayoutViews(const nlohmann::json& j,
                                       json::json_pointer currentPath,
                                       std::shared_ptr<LayoutView> parent)
{
  if (!j.is_array())
  {
    WARN("create layout views from json array, json is not array, return");
    return;
  }

  auto size = j.size();
  for (auto i = 0; i < size; ++i)
  {
    auto path = currentPath;
    path /= i;

    createOneOrMoreLayoutViews(j[i], path, parent);
  }
}

void Layout::Layout::createOneOrMoreLayoutViews(const nlohmann::json& j,
                                                json::json_pointer currentPath,
                                                std::shared_ptr<LayoutView> parent)
{
  if (j.is_object())
  {
    createOneLayoutView(j, currentPath, parent);
  }
  else if (j.is_array())
  {
    createLayoutViews(j, currentPath, parent);
  }
}

void Layout::Layout::collectRules(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    return;
  }

  auto& obj = json["obj"];
  if (obj.is_array())
  {
    for (auto& item : obj)
    {
      auto& id = item[K_ID];
      auto rule = std::make_shared<Internal::Rule::Rule>();
      *rule = item;

      m_rules[id] = rule;
    }
  }
}

void Layout::Layout::configureView(std::shared_ptr<LayoutView> view)
{
  for (auto& child : view->children())
  {
    configureView(child);
  }

  auto nodeId =
    m_designJson[nlohmann::json::json_pointer{ view->path() }].at(K_ID).get<std::string>();
  if (m_rules.find(nodeId) != m_rules.end())
  {
    auto rule = m_rules[nodeId];
    view->configureLayout(
      [rule](std::shared_ptr<VGG::Layout::Internal::Bridge> bridge)
      {
        bridge->flex_node.reset();
        bridge->grid_node.reset();

        if (const auto detail = std::get_if<FlexboxLayout>(&rule->layout))
        {
          auto node = new flexbox_node;
          bridge->flex_node.reset(node);

          node->set_direction(toLibDirecion(detail->direction));
          node->set_align_items(toLibAlignItem(detail->align_items));
          node->set_align_content(toLibAlignContent(detail->align_content));
          node->set_wrap(toLibWrap(detail->wrap));
          node->set_gap(gap_row, detail->row_gap);
          node->set_gap(gap_column, detail->column_gap);
          node->set_padding(padding_top, detail->padding.top);
          node->set_padding(padding_right, detail->padding.right);
          node->set_padding(padding_bottom, detail->padding.bottom);
          node->set_padding(padding_left, detail->padding.left);

          node->set_width(toLibUnit(rule->width.value.types), rule->width.value.value);
          if (rule->max_width.has_value())
          {
            node->set_max_width(toLibUnit(rule->max_width->value.types),
                                rule->max_width->value.value);
          }
          if (rule->min_width.has_value())
          {
            node->set_min_width(toLibUnit(rule->min_width->value.types),
                                rule->min_width->value.value);
          }

          node->set_height(toLibUnit(rule->height.value.types), rule->height.value.value);
          if (rule->max_height.has_value())
          {
            node->set_max_height(toLibUnit(rule->max_height->value.types),
                                 rule->max_height->value.value);
          }
          if (rule->min_height.has_value())
          {
            node->set_min_height(toLibUnit(rule->min_height->value.types),
                                 rule->min_height->value.value);
          }

          if (rule->aspect_ratio.has_value())
          {
            // todo
          }
        }
        else if (const auto detail = std::get_if<GridLayout>(&rule->layout))
        {
          // todo
          // auto node = new grid_layout;
          // bridge->grid_node.reset(node);
        }

        if (const auto detail = std::get_if<FlexboxItem>(&rule->item_in_layout))
        {
          auto node = bridge->flex_node.get();
          if (!node)
          {
            node = new flexbox_node;
            bridge->flex_node.reset(node);

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
        }
        else if (const auto detail = std::get_if<GridItem>(&rule->item_in_layout))
        {
          // todo
        }
      });
  }
}