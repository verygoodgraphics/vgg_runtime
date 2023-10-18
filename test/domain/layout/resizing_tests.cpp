#include "Domain/Layout/Layout.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

using namespace VGG;

class VggResizingTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Layout::Layout> m_sut;

  void SetUp() override
  {
  }

  void TearDown() override
  {
  }

  void setup(const char* fileDir)
  {
    std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                               Helper::RawJsonDocumentBuilder) };
    daruma->load(fileDir);
    m_sut.reset(new Layout::Layout{ daruma->designDoc(), daruma->layoutDoc() });
  }

  auto childFrame(int index)
  {
    auto tree = m_sut->layoutTree();
    auto currentPage = tree->children()[0];
    auto frame = currentPage->children()[index]->frame();
    return frame;
  }
};

TEST_F(VggResizingTestSuite, FigHorizontal)
{
  // Given
  setup("testDataDir/resizing/fig_h/");

  // When
  m_sut->layout(Layout::Size{ 1200, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 20, 0 }, { 100, 40 } },
                                            { { 620, 50 }, { 100, 40 } },
                                            { { 20, 100 }, { 700, 40 } },
                                            { { 320, 150 }, { 100, 40 } },
                                            { { 40, 200 }, { 200, 40 } } };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(childFrame(1) == expectedFrames[1]);
  EXPECT_TRUE(childFrame(2) == expectedFrames[2]);
  EXPECT_TRUE(childFrame(3) == expectedFrames[3]);
  EXPECT_TRUE(childFrame(4) == expectedFrames[4]);
}

TEST_F(VggResizingTestSuite, FigVertical)
{
  // Given
  setup("testDataDir/resizing/fig_v/");

  // When
  m_sut->layout(Layout::Size{ 272, 300 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 0, 20 }, { 40, 100 } },
                                            { { 50, 170 }, { 40, 100 } },
                                            { { 100, 20 }, { 40, 250 } },
                                            { { 150, 95 }, { 40, 100 } },
                                            { { 200, 40 }, { 40, 200 } } };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(childFrame(1) == expectedFrames[1]);
  EXPECT_TRUE(childFrame(2) == expectedFrames[2]);
  EXPECT_TRUE(childFrame(3) == expectedFrames[3]);
  EXPECT_TRUE(childFrame(4) == expectedFrames[4]);
}

TEST_F(VggResizingTestSuite, SketchHorizontal)
{
  // Given
  setup("testDataDir/resizing/sketch_h/");

  // When
  m_sut->layout(Layout::Size{ 800, 500 });

  // Then
  std::vector<Layout::Rect> expectedFrames{
    { { 440, 0 }, { 200, 40 } },          { { 490, 50 }, { 100, 40 } },
    { { 220, 100 }, { 322.222229, 40 } }, { { 495, 150 }, { 225, 40 } },
    { { 220, 200 }, { 500, 40 } },        { { 220, 250 }, { 100, 40 } },
    { { 620, 300 }, { 100, 40 } }
  };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(childFrame(1) == expectedFrames[1]);
  EXPECT_TRUE(childFrame(2) == expectedFrames[2]);
  EXPECT_TRUE(childFrame(3) == expectedFrames[3]);
  EXPECT_TRUE(childFrame(4) == expectedFrames[4]);
  EXPECT_TRUE(childFrame(5) == expectedFrames[5]);
  EXPECT_TRUE(childFrame(6) == expectedFrames[6]);
}

TEST_F(VggResizingTestSuite, SketchVertical)
{
  // Given
  setup("testDataDir/resizing/sketch_v/");

  // When
  m_sut->layout(Layout::Size{ 600, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{
    { { 0, 40 }, { 40, 200 } },          { { 50, 90 }, { 40, 100 } },
    { { 100, 20 }, { 40, 211.111115 } }, { { 150, 53.3333435 }, { 40, 266.666656 } },
    { { 200, 20 }, { 40, 300 } },        { { 250, 20 }, { 40, 100 } },
    { { 300, 220 }, { 40, 100 } }
  };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(childFrame(1) == expectedFrames[1]);
  EXPECT_TRUE(childFrame(2) == expectedFrames[2]);
  EXPECT_TRUE(childFrame(3) == expectedFrames[3]);
  EXPECT_TRUE(childFrame(4) == expectedFrames[4]);
  EXPECT_TRUE(childFrame(5) == expectedFrames[5]);
  EXPECT_TRUE(childFrame(6) == expectedFrames[6]);
}