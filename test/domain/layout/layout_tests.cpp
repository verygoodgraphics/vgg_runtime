#include "base.hpp"

#include "Domain/Layout/Layout.hpp"
#include "Domain/Model/Element.hpp"
#include "UseCase/StartRunning.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

using namespace VGG;

class VggLayoutTestSuite : public BaseVggLayoutTestSuite
{
protected:
  void layout(Layout::Layout& layout, Layout::Size size)
  {
    layout.layout(size, true);
  }
};

TEST_F(VggLayoutTestSuite, Smoke)
{
  // Given
  std::shared_ptr<Daruma> daruma{
    new Daruma(Helper::RawJsonDocumentBuilder, Helper::RawJsonDocumentBuilder)
  };
  std::string filePath = "testDataDir/layout/0_space_between/";
  daruma->load(filePath);

  // When
  Layout::Layout sut{ daruma->designDoc(), daruma->layoutDoc() };

  // Then
}

TEST_F(VggLayoutTestSuite, SpaceBetween)
{
  // Given
  setupWithExpanding("testDataDir/layout/0_space_between/");

  // When
  layout(*m_sut, Layout::Size{ 1400, 900 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 0, 0 }, { 200, 150 } },
                                            { { 1200, 0 }, { 200, 250 } } };
  EXPECT_TRUE(descendantFrame({ 0 }) == expectedFrames[0]);
  EXPECT_TRUE(descendantFrame({ 1 }) == expectedFrames[1]);
}

TEST_F(VggLayoutTestSuite, Wrap)
{
  // Given
  setupWithExpanding("testDataDir/layout/1_wrap/");

  // When
  layout(*m_sut, Layout::Size{ 799, 900 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 0, 0 }, { 200, 150 } },
                                            { { 0, 310 }, { 200, 200 } } };
  EXPECT_TRUE(descendantFrame({ 0 }) == expectedFrames[0]);
  EXPECT_TRUE(descendantFrame({ 3 }) == expectedFrames[1]);
}

TEST_F(VggLayoutTestSuite, LayoutInstance)
{
  // Given
  setupWithExpanding("testDataDir/layout/3_flex_with_symbol_instance/");

  // When

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 10, 10 }, { 100, 200 } },
                                            { { 590, 10 }, { 200, 100 } } };
  EXPECT_TRUE(descendantFrame({ 0, 0 }) == expectedFrames[0]);
  EXPECT_TRUE(descendantFrame({ 0, 1 }) == expectedFrames[1]);
}

TEST_F(VggLayoutTestSuite, FrameOfNodeWhoseParentPositionInBoundsIsNotZero)
{
  // Given
  std::shared_ptr<Daruma> daruma{
    new Daruma(Helper::RawJsonDocumentBuilder, Helper::RawJsonDocumentBuilder)
  };
  std::string filePath = "testDataDir/layout/21_layout_node_frame/";
  daruma->load(filePath);

  // When
  Layout::Layout sut{ daruma->designDoc(), daruma->layoutDoc() };

  // Then
  auto tree = sut.layoutTree();
  auto page = tree->children()[0];

  auto         child1Frame = page->children()[1]->frame();
  Layout::Rect expectChild0Frame{ { 0, 0 }, { 200, 100 } };
  EXPECT_TRUE(child1Frame == expectChild0Frame);
}

TEST_F(VggLayoutTestSuite, ApplyLayoutToViewHierarchy)
{
  /*
  Apply layout to container and items only.

  ---
  Given:
    node: A -> B -> C
    A is a flex container
    B is a flex item
    C is another flex container
  When:
    calculate layout for node A
  Then:
    apply layout to container A and item B, not C

  ---
  Given:
    node: A -> B -> C
    A is a flex container
    B is a flex item and flex container
    C is a flex item of B
  When:
    calculate layout for node A
  Then:
    apply layout to node A, B, and C
  */

  // Given
  setupWithExpanding("testDataDir/layout/101_self_and_grandson_layout/");

  // When

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 24, 14 }, { 104, 27 } } };
  EXPECT_TRUE(descendantFrame({ 0, 1, 0 }) == expectedFrames[0]);
}

TEST_F(VggLayoutTestSuite, AbosoluteFlexItem)
{
  // Given
  setupWithExpanding("testDataDir/layout/31_absolute_flex_item/");

  // When

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 233.99996948242188, 24 },
                                              { 40.000038146972656, 48 } } };
  EXPECT_TRUE(descendantFrame({ 0, 1 }) == expectedFrames[0]);
}

TEST_F(VggLayoutTestSuite, RotatedLine)
{
  // Given
  setupWithExpanding("testDataDir/layout/201_rotated_line/");

  // When
  m_sut->layout(Layout::Size{ 1920, 1080 });

  // Then
  {
    std::vector<Layout::Rect> expectedFrames{ { { 172, 0 }, { 105, 4 } } };
    EXPECT_TRUE(descendantFrame({ 1 }, 0) == expectedFrames[0]);
  }
  {
    std::vector<Layout::Rect> expectedFrames{ { { 0, 105 }, { 4, 228 } } };
    EXPECT_TRUE(descendantFrame({ 1 }, 1) == expectedFrames[0]);
  }
  {
    std::vector<Layout::Rect> expectedFrames{ { { 172, 0 }, { 90, 4 } } };
    EXPECT_TRUE(descendantFrame({ 1 }, 2) == expectedFrames[0]);
  }
  {
    std::vector<Layout::Rect> expectedFrames{ { { 0, 105 }, { 4, 200 } } };
    EXPECT_TRUE(descendantFrame({ 1 }, 3) == expectedFrames[0]);
  }
}

TEST_F(VggLayoutTestSuite, AddAutoMarginToItemsInNoWrapSpaceBetweenFlexContainer)
{
  // Given
  setupWithExpanding("testDataDir/layout/202_auto_margin/");

  // When
  m_sut->layout(Layout::Size{ 380, 300 }, true);

  // Then
  {
    std::vector<Layout::Rect> expectedFrames{ { { 180, 0 }, { 200, 100 } } };
    EXPECT_TRUE(descendantFrame({ 1 }, 0) == expectedFrames[0]);
  }
}

TEST_F(VggLayoutTestSuite, NonZeroFrameOrigin)
{
  // Given
  setupWithExpanding("testDataDir/layout/203_frame_with_non_zero_origin/");

  // When
  layout(*m_sut, Layout::Size{ 1920, 1080 });

  // Then
  std::vector<Layout::Rect> expectedBounds{ { { -742, -477 }, { 1920, 1080 } } };

  auto bounds = firstPage()->bounds();
  EXPECT_TRUE(bounds == expectedBounds[0]);
}

TEST_F(VggLayoutTestSuite, UpdateFlexContainerWidthByConstraints)
{
  // Given
  setupWithExpanding("testDataDir/layout/204_resize_flex_container/");

  // When
  layout(*m_sut, Layout::Size{ 1920, 1080 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 94, 101 }, { 1762, 342 } } };

  EXPECT_TRUE(descendantFrame({ 0 }, 0) == expectedFrames[0]);
}

TEST_F(VggLayoutTestSuite, EmptyContainer)
{
  // Given
  setupWithExpanding("testDataDir/layout/205_empty_container/");

  // When
  layout(*m_sut, Layout::Size{ 1920, 1080 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 26, 0 }, { 6, 6 } } };

  EXPECT_TRUE(descendantFrame({ 1 }, 0) == expectedFrames[0]);
}

TEST_F(VggLayoutTestSuite, FirstOnTop)
{
  setupWithExpanding("testDataDir/layout/11_first_on_top/");

  auto tree = m_sut->designDocTree();
  EXPECT_TRUE(tree->children().front()->isFirstOnTop());

  auto json = m_sut->displayDesignDoc()->content();
  EXPECT_EQ(json["frames"][0]["childObjects"][0]["name"], "Rectangle 3");
}
