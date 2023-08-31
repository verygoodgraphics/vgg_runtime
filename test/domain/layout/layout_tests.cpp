#include "Domain/Layout/Layout.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

using namespace VGG;

class VggLayoutTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Layout::Layout> m_sut;

  void SetUp() override
  {
  }

  void TearDown() override
  {
    m_sut.reset();
  }
};

TEST_F(VggLayoutTestSuite, Smoke)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string file_path = "testDataDir/layout/0_space_between/";
  daruma->load(file_path);

  // When
  Layout::Layout sut{ daruma };

  // Then
}

TEST_F(VggLayoutTestSuite, Layout)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string file_path = "testDataDir/layout/0_space_between/";
  daruma->load(file_path);

  Layout::Layout sut{ daruma };
  Layout::Size window_size{ 1400, 900 };

  // When
  sut.layout(window_size);

  // Then
  auto tree = sut.layoutTree();
  auto current_page = tree->children()[0];

  auto left_child_frame = current_page->children()[0]->frame();
  Layout::Rect expect_left_child_frame{ { 100, 40 }, { 200, 150 } };
  EXPECT_TRUE(left_child_frame == expect_left_child_frame);

  auto right_child_frame = current_page->children()[1]->frame();
  Layout::Rect expect_right_child_frame{ { 1100, 40 }, { 200, 250 } };
  EXPECT_TRUE(right_child_frame == expect_right_child_frame);
}

TEST_F(VggLayoutTestSuite, GridWrap)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                             Helper::RawJsonDocumentBuilder) };
  std::string filePath = "testDataDir/layout/1_grid_wrap/";
  daruma->load(filePath);

  Layout::Layout sut{ daruma };
  Layout::Size windowSize{ 1400, 900 };

  // When
  sut.layout(windowSize);

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