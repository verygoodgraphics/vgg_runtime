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
  // todo

  // setup tree
  auto formPtr = form.get();
  auto frame1Ptr = frame1.get();
  auto textPtr = text.get();

  frame1->add_child(text, 0);
  form->add_child(frame1, 0);

  frame1Ptr->calc_layout();
  formPtr->calc_layout();

  EXPECT_DOUBLE_EQ(237.0, frame1Ptr->get_layout_width());
}