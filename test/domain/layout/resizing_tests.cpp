#include "Domain/Layout/Layout.hpp"
#include "UseCase/StartRunning.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

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

  auto setupModel(const char* fileDir)
  {
    std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder,
                                               Helper::RawJsonDocumentBuilder) };
    daruma->load(fileDir);
    return daruma;
  }

  void setup(const char* fileDir)
  {
    auto daruma = setupModel(fileDir);
    m_sut.reset(new Layout::Layout{ daruma->designDoc(), daruma->layoutDoc() });
  }

  void setupWithExpanding(const char* fileDir)
  {
    auto daruma = setupModel(fileDir);

    StartRunning startRunning{ daruma };
    m_sut = startRunning.layout();
  }

  auto firstPage()
  {
    auto tree = m_sut->layoutTree();
    return tree->children()[0];
  }

  auto childFrame(int index)
  {
    auto frame = firstPage()->children()[index]->frame();
    return frame;
  }

  auto grandsonFrame(int index, int index2)
  {
    auto frame = firstPage()->children()[index]->children()[index2]->frame();
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

TEST_F(VggResizingTestSuite, ExpandingSymbol)
{
  // Given
  setupWithExpanding("testDataDir/resizing/fig_instance_width/");

  // When

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 20, 0 }, { 100, 40 } },
                                            { { 620, 50 }, { 100, 40 } },
                                            { { 20, 100 }, { 700, 40 } },
                                            { { 320, 150 }, { 100, 40 } },
                                            { { 39.9999962, 200 }, { 199.999985, 40 } } };
  EXPECT_TRUE(grandsonFrame(1, 0) == expectedFrames[0]);
  EXPECT_TRUE(grandsonFrame(1, 1) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(1, 2) == expectedFrames[2]);
  EXPECT_TRUE(grandsonFrame(1, 3) == expectedFrames[3]);
  EXPECT_TRUE(grandsonFrame(1, 4) == expectedFrames[4]);
}

TEST_F(VggResizingTestSuite, FigGroup)
{
  // Given
  setup("testDataDir/resizing/fig_group/");

  // When
  m_sut->layout(Layout::Size{ 600, 600 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 89, 45 }, { 440, 500 } },
                                            { { 0, 0 }, { 100, 40 } },
                                            { { 400, 400 }, { 40, 100 } } };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(grandsonFrame(0, 0) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(0, 1) == expectedFrames[2]);
}

TEST_F(VggResizingTestSuite, FigUnion)
{
  // Given
  setup("testDataDir/resizing/fig_union/");

  // When
  m_sut->layout(Layout::Size{ 600, 600 });

  // Then
  std::vector<Layout::Rect> expectedFrames{
    { { 20, 20 }, { 220, 280 } },   { { 0, 0 }, { 200, 140 } }, { { 80, 80 }, { 140, 200 } },
    { { 300, 300 }, { 240, 250 } }, { { 0, 0 }, { 100, 40 } },  { { 200, 150 }, { 40, 100 } }
  };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(grandsonFrame(0, 0) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(0, 1) == expectedFrames[2]);

  EXPECT_TRUE(childFrame(1) == expectedFrames[3]);
  EXPECT_TRUE(grandsonFrame(1, 0) == expectedFrames[4]);
  EXPECT_TRUE(grandsonFrame(1, 1) == expectedFrames[5]);
}

TEST_F(VggResizingTestSuite, FigNestedGroup)
{
  // Given
  setup("testDataDir/resizing/fig_nested_group/");

  // When
  m_sut->layout(Layout::Size{ 800, 800 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 89, 27 }, { 431, 473.999969 } },
                                            { { 0, 0 }, { 431, 473.999969 } },
                                            { { 366, 418 }, { 51, 43 } } };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(grandsonFrame(0, 0) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(0, 1) == expectedFrames[2]);
}

TEST_F(VggResizingTestSuite, FigGroupWithRotatedChild)
{
  // Given
  setup("testDataDir/resizing/fig_group_with_rotated_child/");

  // When
  m_sut->layout(Layout::Size{ 800, 800 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 89, 45 },
                                              { 655.8948364257813, 703.8250122070313 } },
                                            { { 0, 0 }, { 100, 40 } },
                                            { { 600, 600 }, { 40, 100 } },
                                            { { 584.105103, 609.855774 }, { 40, 100 } } };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(grandsonFrame(0, 0) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(0, 1) == expectedFrames[2]);
  EXPECT_TRUE(grandsonFrame(0, 2) == expectedFrames[3]);
}

TEST_F(VggResizingTestSuite, GetAffineTransform)
{
  // Given
  const glm::mat3x2 glmMatrix{ 0.9396926164627075, 0.3420201241970062, -0.3420201241970062,
                               0.9396926164627075, 177.10513305664062, -228.8557586669922 };
  const glm::mat3 oldGlmPoints{ 0, 0, 1, 0, 40, 1, 100, 0, 1 };
  const auto newGlmPoints = glmMatrix * oldGlmPoints;

  const std::array<Layout::Point, 3> oldPoints{
    Layout::Point{ oldGlmPoints[0].x, oldGlmPoints[0].y },
    Layout::Point{ oldGlmPoints[1].x, oldGlmPoints[1].y },
    Layout::Point{ oldGlmPoints[2].x, oldGlmPoints[2].y }
  };
  const std::array<Layout::Point, 3> newPoints{
    Layout::Point{ newGlmPoints[0].x, newGlmPoints[0].y },
    Layout::Point{ newGlmPoints[1].x, newGlmPoints[1].y },
    Layout::Point{ newGlmPoints[2].x, newGlmPoints[2].y }
  };

  // When
  const auto computedMatrix = Layout::getAffineTransform(oldPoints, newPoints);

  // Then
  Layout::Matrix layoutMatrix{ glmMatrix[0].x, glmMatrix[0].y, glmMatrix[1].x,
                               glmMatrix[1].y, glmMatrix[2].x, glmMatrix[2].y };
  EXPECT_TRUE(layoutMatrix == computedMatrix);
}