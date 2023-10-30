#include "base.hpp"

#include "Domain/Layout/Layout.hpp"
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
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/0_space_between/";
  daruma->load(filePath);

  // When
  Layout::Layout sut{ daruma->designDoc(), daruma->layoutDoc() };

  // Then
}

TEST_F(VggLayoutTestSuite, Layout)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/0_space_between/";
  daruma->load(filePath);

  Layout::Layout sut{ daruma->designDoc(), daruma->layoutDoc() };
  Layout::Size windowSize{ 1400, 900 };

  // When
  layout(sut, windowSize);

  // Then
  auto tree = sut.layoutTree();
  auto currentPage = tree->children()[0];

  auto leftChildFrame = currentPage->children()[0]->frame();
  Layout::Rect expectedLeftChildFrame{ { 100, 40 }, { 200, 150 } };
  EXPECT_TRUE(leftChildFrame == expectedLeftChildFrame);

  auto rightChildFrame = currentPage->children()[1]->frame();
  Layout::Rect expectedRightChildFrame{ { 1100, 40 }, { 200, 250 } };
  EXPECT_TRUE(rightChildFrame == expectedRightChildFrame);
}

TEST_F(VggLayoutTestSuite, GridWrap)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/1_grid_wrap/";
  daruma->load(filePath);

  Layout::Layout sut{ daruma->designDoc(), daruma->layoutDoc() };
  Layout::Size windowSize{ 1400, 900 };

  // When
  layout(sut, windowSize);

  // Then
  auto tree = sut.layoutTree();
  auto currentPage = tree->children()[0];

  auto gridContainer = currentPage;

  auto child0Frame = gridContainer->children()[0]->frame();
  Layout::Rect expectChild0Frame{ { 100, 40 }, { 200, 250 } };
  EXPECT_TRUE(child0Frame == expectChild0Frame);

  auto child3Frame = gridContainer->children()[3]->frame();
  Layout::Rect expectChild3Frame{ { 1060, 40 }, { 200, 200 } };
  EXPECT_TRUE(child3Frame == expectChild3Frame);
}

TEST_F(VggLayoutTestSuite, ScaleSubtree)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/3_flex_with_symbol_instance/";
  daruma->load(filePath);

  StartRunning startRunning{ daruma };
  auto sut = startRunning.layout();
  Layout::Size windowSize{ 1400, 900 };

  // When
  layout(*sut, windowSize);

  // Then
  auto tree = sut->layoutTree();
  auto currentPage = tree->children()[1];

  auto container = currentPage->children()[0];

  auto child0Frame = container->children()[0]->frame();
  Layout::Rect expectChild0Frame{ { 33, 40 }, { 330, 400 } };
  EXPECT_TRUE(child0Frame == expectChild0Frame);

  auto child1Frame = container->children()[1]->frame();
  Layout::Rect expectChild1Frame{ { 495, 40 }, { 660, 200 } };
  EXPECT_TRUE(child1Frame == expectChild1Frame);
}

TEST_F(VggLayoutTestSuite, FrameOfNodeWhoseParentPositionInBoundsIsNotZero)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/21_layout_node_frame/";
  daruma->load(filePath);

  // When
  Layout::Layout sut{ daruma->designDoc(), daruma->layoutDoc() };

  // Then
  auto tree = sut.layoutTree();
  auto page = tree->children()[0];

  auto child1Frame = page->children()[1]->frame();
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