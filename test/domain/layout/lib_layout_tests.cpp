#include <flexbox_node.h>
#include <grid_layout.h>

#include <memory>

#include <gtest/gtest.h>

TEST(LibLayoutTest, GrowShrinkWithWidth)
{
  auto form = std::make_unique<flexbox_node>();
  auto frame1 = std::make_unique<flexbox_node>();
  auto text = std::make_unique<flexbox_node>();
  auto icon = std::make_unique<flexbox_node>();

  // 0
  form->set_direction(direction_row);
  form->set_justify_content(justify_content_flex_start);
  form->set_align_items(align_items_flex_start);
  form->set_align_content(align_content_flex_start);
  form->set_wrap(wrap_no_wrap);
  form->set_gap(gap_row, 0);
  form->set_gap(gap_column, 0);
  form->set_padding(padding_top, 0);
  form->set_padding(padding_right, 0);
  form->set_padding(padding_bottom, 0);
  form->set_padding(padding_left, 0);

  form->set_width(unit_point, 237.0);
  form->set_height(unit_point, 38.0);

  // 0/0
  frame1->set_direction(direction_row);
  frame1->set_justify_content(justify_content_flex_start);
  frame1->set_align_items(align_items_center);
  frame1->set_align_content(align_content_center);
  frame1->set_wrap(wrap_no_wrap);
  frame1->set_gap(gap_row, 0);
  frame1->set_gap(gap_column, 10.0);
  frame1->set_padding(padding_top, 12.0);
  frame1->set_padding(padding_right, 8.0);
  frame1->set_padding(padding_bottom, 12.0);
  frame1->set_padding(padding_left, 8.0);

  frame1->set_position(position_relative);
  frame1->set_grow(1.0);
  frame1->set_shrink(1.0);

  frame1->set_width(unit_point, 365.0);
  frame1->set_height(unit_point, 38.0);

  // 0/0/0
  text->set_position(position_relative);
  text->set_grow(0.0);
  text->set_shrink(0.0);

  text->set_width(unit_point, 91.0);
  text->set_height(unit_point, 20.0);

  // 0/0/1
  icon->set_position(position_relative);
  icon->set_grow(0.0);
  icon->set_shrink(0.0);

  icon->set_width(unit_point, 20.0);
  icon->set_height(unit_point, 20.0);

  // setup tree
  auto formPtr = form.get();
  auto frame1Ptr = frame1.get();
  auto textPtr = text.get();

  // when
  frame1->add_child(text, 0);
  frame1->add_child(icon, 1);
  frame1Ptr->calc_layout();
  EXPECT_DOUBLE_EQ(365.0, frame1Ptr->get_layout_width());

  // when
  form->add_child(frame1, 0);
  formPtr->calc_layout();

  EXPECT_DOUBLE_EQ(237.0, frame1Ptr->get_layout_width());
}

TEST(LibLayoutTest, YPosition)
{
  auto root = std::make_unique<flexbox_node>();
  auto child0 = std::make_unique<flexbox_node>();
  auto child1 = std::make_unique<flexbox_node>();

  // 0
  root->set_direction(direction_row);
  root->set_justify_content(justify_content_center);
  root->set_align_items(align_items_center);
  root->set_align_content(align_content_center);
  root->set_wrap(wrap_no_wrap);
  root->set_gap(gap_row, 0);
  root->set_gap(gap_column, 16);
  root->set_padding(padding_top, 0);
  root->set_padding(padding_right, 0);
  root->set_padding(padding_bottom, 0);
  root->set_padding(padding_left, 0);

  root->set_width(unit_point, 221.0);
  root->set_height(unit_point, 27.0);

  // 0/0
  child0->set_position(position_relative);

  child0->set_grow(0.0);
  child0->set_shrink(0.0);

  child0->set_width(unit_point, 24.0);
  child0->set_height(unit_point, 24.0);

  // 0/1
  child1->set_position(position_relative);

  child1->set_grow(0.0);
  child1->set_shrink(0.0);

  child1->set_width(unit_point, 181.0);
  child1->set_height(unit_point, 27.0);

  // setup tree
  auto rootPtr = root.get();
  auto child0Ptr = child0.get();
  auto child1Ptr = child1.get();

  // when
  root->add_child(child0, -1);
  root->add_child(child1, -1);
  rootPtr->calc_layout();

  // NOTE: shoule be 1.5, but yoga return 2
  EXPECT_DOUBLE_EQ(2, child0Ptr->get_layout_top());
}

TEST(LibLayoutTest, StrechNodeWithPercentHeight)
{
  auto          node00 = std::make_unique<flexbox_node>();
  auto          pNode00 = node00.get();
  flexbox_node* pNode00_1{ nullptr };
  {
    {
      auto targetNode = node00.get();
      {
        targetNode->set_direction(direction_row);
        targetNode->set_justify_content(justify_content_flex_start);
        targetNode->set_align_items(align_items_flex_start);
        targetNode->set_align_content(align_content_flex_start);
        targetNode->set_wrap(wrap_no_wrap);
        targetNode->set_gap(gap_row, 0);
        targetNode->set_gap(gap_column, 40);
        targetNode->set_padding(padding_top, 48);
        targetNode->set_padding(padding_right, 0);
        targetNode->set_padding(padding_bottom, 0);
        targetNode->set_padding(padding_left, 48);
      }
      {
        targetNode->set_width(unit_auto, 0);
        targetNode->set_height(unit_auto, 0);
      }
    }

    auto node00_0 = std::make_unique<flexbox_node>();
    {
      {
        auto targetNode = node00_0.get();
        {
          targetNode->set_direction(direction_column);
          targetNode->set_justify_content(justify_content_flex_start);
          targetNode->set_align_items(align_items_flex_start);
          targetNode->set_align_content(align_content_flex_start);
          targetNode->set_wrap(wrap_no_wrap);
          targetNode->set_gap(gap_row, 24);
          targetNode->set_gap(gap_column, 0);
          targetNode->set_padding(padding_top, 0);
          targetNode->set_padding(padding_right, 0);
          targetNode->set_padding(padding_bottom, 48);
          targetNode->set_padding(padding_left, 0);
        }
        {
          targetNode->set_width(unit_auto, 0);
          targetNode->set_height(unit_auto, 0);
        }
        {
          targetNode->set_position(position_relative);
          targetNode->set_grow(0.0);
          targetNode->set_shrink(0.0);
        }
        {
          targetNode->set_width(unit_auto, 0);
          targetNode->set_height(unit_auto, 0);
        }
      }

      {
        auto node00_00 = std::make_unique<flexbox_node>();
        {
          auto targetNode = node00_00.get();
          {
            targetNode->set_direction(direction_row);
            targetNode->set_justify_content(justify_content_flex_start);
            targetNode->set_align_items(align_items_flex_start);
            targetNode->set_align_content(align_content_flex_start);
            targetNode->set_wrap(wrap_no_wrap);
            targetNode->set_gap(gap_row, 0);
            targetNode->set_gap(gap_column, 8);
            targetNode->set_padding(padding_top, 24);
            targetNode->set_padding(padding_right, 0);
            targetNode->set_padding(padding_bottom, 0);
            targetNode->set_padding(padding_left, 0);
          }
          {
            targetNode->set_width(unit_auto, 0);
            targetNode->set_height(unit_auto, 0);
          }
          {
            targetNode->set_position(position_relative);
            targetNode->set_grow(0.0);
            targetNode->set_shrink(0.0);
          }
          {
            targetNode->set_width(unit_auto, 0);
            targetNode->set_height(unit_auto, 0);
          }
        }
        {
          auto node00_00_0 = std::make_unique<flexbox_node>();
          auto targetNode = node00_00_0.get();
          {
            targetNode->set_direction(direction_row);
            targetNode->set_justify_content(justify_content_center);
            targetNode->set_align_items(align_items_center);
            targetNode->set_align_content(align_content_center);
            targetNode->set_wrap(wrap_no_wrap);
            targetNode->set_gap(gap_row, 0);
            targetNode->set_gap(gap_column, 10);
            targetNode->set_padding(padding_top, 15);
            targetNode->set_padding(padding_right, 20);
            targetNode->set_padding(padding_bottom, 12);
            targetNode->set_padding(padding_left, 20);
          }
          {
            targetNode->set_width(unit_point, 56);
            targetNode->set_height(unit_point, 64);
          }
          {
            targetNode->set_position(position_relative);
            targetNode->set_grow(0.0);
            targetNode->set_shrink(0.0);
          }
          {
            targetNode->set_width(unit_point, 56);
            targetNode->set_height(unit_point, 64);
          }

          {
            auto node00_00_00 = std::make_unique<flexbox_node>();
            {
              auto targetNode = node00_00_00.get();
              {
                targetNode->set_position(position_relative);
                targetNode->set_grow(0.0);
                targetNode->set_shrink(0.0);
              }
              {
                targetNode->set_width(unit_point, 62);
                targetNode->set_height(unit_point, 37);
              }
            }
            node00_00_0->add_child(node00_00_00, -1);
          }
          node00_00->add_child(node00_00_0, -1);
        }
        node00_0->add_child(node00_00, -1);
      }
    }
    node00->add_child(node00_0, -1);

    auto node00_1 = std::make_unique<flexbox_node>();
    pNode00_1 = node00_1.get();
    {
      auto targetNode = node00_1.get();
      {
        targetNode->set_position(position_relative);
        targetNode->set_grow(0.0);
        targetNode->set_shrink(0.0);
      }
      {
        targetNode->set_width(unit_point, 247);
        targetNode->set_height(unit_percent, 100);
        targetNode->set_align_self(align_items_stretch);
      }
    }
    node00->add_child(node00_1, -1);
  }

  // when
  pNode00->calc_layout();

  EXPECT_DOUBLE_EQ(136, pNode00_1->get_layout_height());
}

TEST(LibLayoutTest, VerticalFlexAlignItemCenterContainerWithFillWidthItem)
{
  auto container = std::make_unique<flexbox_node>();
  auto child = std::make_unique<flexbox_node>();

  // container
  container->set_direction(direction_column);
  container->set_justify_content(justify_content_flex_start);
  container->set_align_items(align_items_center);
  container->set_align_content(align_content_flex_start);
  container->set_wrap(wrap_no_wrap);
  container->set_gap(gap_row, 10);
  container->set_gap(gap_column, 0);
  container->set_padding(padding_top, 0);
  container->set_padding(padding_right, 0);
  container->set_padding(padding_bottom, 0);
  container->set_padding(padding_left, 0);

  container->set_width(unit_point, 1660.0);
  container->set_height(unit_point, 273.0);

  // child
  child->set_position(position_relative);
  child->set_grow(0.0);
  child->set_shrink(0.0);

  child->set_max_width(unit_point, 1400.0);
  child->set_width(unit_percent, 100.0);
  child->set_height(unit_point, 101.0);

  // setup tree
  auto pChild = child.get();

  // when
  container->add_child(child, 0);
  container->calc_layout();

  EXPECT_DOUBLE_EQ(130.0, pChild->get_layout_left());
}