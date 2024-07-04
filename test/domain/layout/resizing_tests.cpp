#include "base.hpp"

#include "Domain/Layout/Math.hpp"
#include "Math/Algebra.hpp"

#include "test_config.hpp"

#include <gtest/gtest.h>

#include <glm/gtx/matrix_transform_2d.hpp>

class VggResizingTestSuite : public BaseVggLayoutTestSuite
{
};

TEST_F(VggResizingTestSuite, FigHorizontal)
{
  // Given
  setup("testDataDir/resizing/fig_h/");

  // When
  layoutAllPage(Layout::Size{ 1200, 400 });

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
  layoutAllPage(Layout::Size{ 272, 300 });

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
  layoutAllPage(Layout::Size{ 800, 500 });

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
  layoutAllPage(Layout::Size{ 600, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{
    { { 0, 40 }, { 40, 200 } },
    { { 50, 90 }, { 40, 100 } },
    { { 100, 20 }, { 40, 211.111115 } },
    { { 150, 53.333332061767578 }, { 40, 266.66665649414063 } },
    { { 200, 20 }, { 40, 300 } },
    { { 250, 20 }, { 40, 100 } },
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
  layoutAllPage(Layout::Size{ 600, 600 });

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
  layoutAllPage(Layout::Size{ 600, 600 });

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
  layoutAllPage(Layout::Size{ 800, 800 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 89, 27 }, { 431, 473.99996948242188 } },
                                            { { 0, 0 }, { 431, 473.99996948242188 } },
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
  layoutAllPage(Layout::Size{ 800, 800 });

  // Then
  std::vector<Layout::Rect> expectedFrames{
    { { 89, 45 }, { 655.89483642578125, 703.82501220703125 } },
    { { 0, 0 }, { 100, 40 } },
    { { 584.10510215593126, 609.85577287313663 }, { 40, 100 } }
  };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]);
  EXPECT_TRUE(grandsonFrame(0, 0) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(0, 1) == expectedFrames[2]);
}

TEST_F(VggResizingTestSuite, FigGroupWithRotatedScaleChild)
{
  // Given
  setup("testDataDir/resizing/fig_group_with_rotated_scale_child/");

  // When
  layoutAllPage(Layout::Size{ 800, 600 });

  // Then
  std::vector<Layout::Rect> expectedFrames{
    { { 89, 45 }, { 639.90191650390625, 531.833740234375 } },
    { { 0, 0 }, { 100, 40 } },
    { { 401.10248921214236, 350.21069997189215 }, { 112.78665924072266, 237.09202575683594 } }
  };
  EXPECT_TRUE(childFrame(0) == expectedFrames[0]); // Group's frame is different from figma because
                                                   // the rotation angle of the child is different.
  EXPECT_TRUE(grandsonFrame(0, 0) == expectedFrames[1]);
  EXPECT_TRUE(grandsonFrame(0, 1) == expectedFrames[2]); // Child's frame is different from figma
                                                         // because the rotation angle is different
}

TEST_F(VggResizingTestSuite, GetAffineTransform)
{
  // Given
  const glm::mat3x2 glmMatrix{ 0.9396926164627075, 0.3420201241970062, -0.3420201241970062,
                               0.9396926164627075, 177.10513305664062, -228.8557586669922 };
  const glm::mat3   oldGlmPoints{ 0, 0, 1, 0, 40, 1, 100, 0, 1 };
  const auto        newGlmPoints = glmMatrix * oldGlmPoints;

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
  const auto computedMatrix = Layout::Matrix::getAffineTransform(oldPoints, newPoints);

  // Then
  Layout::Matrix layoutMatrix{ glmMatrix[0].x, glmMatrix[0].y, glmMatrix[1].x,
                               glmMatrix[1].y, glmMatrix[2].x, glmMatrix[2].y };
  EXPECT_TRUE(layoutMatrix == computedMatrix);
}

TEST_F(VggResizingTestSuite, AdjustChildPostionIfTheChildSizeChanged)
{
  // Given
  setupWithExpanding("testDataDir/resizing/child_size_changed/");

  // When

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 147.5, 14.0 }, { 221.0, 27.0 } },
                                            { { 0, 2 }, { 24, 24 } },
                                            { { 40, 0 }, { 181, 27 } },
                                            { { 139.0, 14.0 }, { 239.0, 27.0 } } };
  EXPECT_TRUE(descendantFrame({ 0, 0 }) == expectedFrames[0]);
  EXPECT_TRUE(descendantFrame({ 0, 0, 0 }) == expectedFrames[1]);
  EXPECT_TRUE(descendantFrame({ 0, 0, 1 }) == expectedFrames[2]);
  EXPECT_TRUE(descendantFrame({ 2, 0 }) == expectedFrames[3]);
}

TEST_F(VggResizingTestSuite, ResizeMatrix)
{
  SKIP_DEBUG_TEST;

  // Given
  const glm::mat3x2 glmMatrix{ 0.7660444378852844, 0.6427876353263855, -0.6427876353263855,
                               0.7660444378852844, 252.23641967773438, -327.93597412109375 };
  const glm::mat3   t1{ 0.7660444378852844,  0.6427876353263855,  0,
                      -0.6427876353263855, 0.7660444378852844,  0,
                      252.23641967773438,  -327.93597412109375, 1 };
  const glm::mat3   oldGlmPoints{
    56.64102554321289, 0.0, 1, 38.6410243309369, -49.000003814697266, 1, 0.0, -32.25000251069361, 1
  };
  {
    const auto glmPoints = glmMatrix * oldGlmPoints;
    auto       left = std::min(std::min(glmPoints[0].x, glmPoints[1].x), glmPoints[2].x);
    auto       top = std::max(std::max(glmPoints[0].y, glmPoints[1].y), glmPoints[2].y);

    auto t = glm::translate(glm::mat3{ 1 }, glm::vec2{ -left, -top });
    t = glm::scale(t, glm::vec2{ 2, 1 });
    t = glm::translate(t, glm::vec2{ left / 2, top });
    t *= t1;

    const glm::mat3x2 t2{ t[0].x, t[0].y, t[1].x, t[1].y, t[2].x, t[2].y };
    auto              p1 = oldGlmPoints;
    const glm::mat3   p2{ p1[0].x, p1[0].y, 1, p1[1].x, p1[1].y, 1, p1[2].x, p1[2].y, 1 };
    auto              p3 = t2 * p2; // good
    auto              _ = 1 + 1;
  }
  {
    const auto glmPoints = glmMatrix * oldGlmPoints;
    auto       left = std::min(std::min(glmPoints[0].x, glmPoints[1].x), glmPoints[2].x);
    auto       top = std::max(std::max(glmPoints[0].y, glmPoints[1].y), glmPoints[2].y);

    auto t = glm::translate(glm::mat3{ 1 }, glm::vec2{ -left, -top });
    t = glm::scale(t, glm::vec2{ 2, 1 });
    t = glm::translate(t, glm::vec2{ left / 2, top });

    const glm::mat3x2 t2{ t[0].x, t[0].y, t[1].x, t[1].y, t[2].x, t[2].y };
    auto              p1 = glmPoints;
    const glm::mat3   p2{ p1[0].x, p1[0].y, 1, p1[1].x, p1[1].y, 1, p1[2].x, p1[2].y, 1 };
    auto              p3 = t2 * p2; // good
    auto              _ = 1 + 1;
  }
  {
    const auto glmPoints = glmMatrix * oldGlmPoints;
    auto       left = std::min(std::min(glmPoints[0].x, glmPoints[1].x), glmPoints[2].x);
    auto       bottom = std::min(std::min(glmPoints[0].y, glmPoints[1].y), glmPoints[2].y);

    auto t = glm::translate(glm::mat3{ 1 }, glm::vec2{ -left, -bottom });
    t = glm::scale(t, glm::vec2{ 2, 1 });
    t = glm::translate(t, glm::vec2{ left / 2, bottom });

    const glm::mat3x2 t2{ t[0].x, t[0].y, t[1].x, t[1].y, t[2].x, t[2].y };
    auto              p1 = glmPoints;
    const glm::mat3   p2{ p1[0].x, p1[0].y, 1, p1[1].x, p1[1].y, 1, p1[2].x, p1[2].y, 1 };
    auto              p3 = t2 * p2; // good
    auto              _ = 1 + 1;
  }
  {
    auto t = glm::rotate(glm::mat3{ 1 }, 40.f);
    auto _ = 1 + 1;
  }
  {
    const glm::mat3 resultPoints{ 623.64, 206.47, 1, 522.56, 333.8, 1, 352.12, 271.76, 1 };
    const glm::mat3 flipYMatrix{ 1, 0, 0, 0, -1, 0, 0, 0, 1 };
    const glm::mat3 oldMatrix{ 0.7660444507738347, -0.6427876005638363, 0,
                               0.6427876005638363, 0.7660444507738347,  0,
                               264.303218818517,   -166.59799083442599, 1 };

    // auto points = resultPoints * flipYMatrix * glm::inverse(oldMatrix);
    auto t = glm::rotate(glm::mat3{ 1 }, glm::radians(40.f)); // fig
    auto points = resultPoints * flipYMatrix * t;

    auto t2 = glm::rotate(glm::mat3{ 1 }, glm::radians(-40.f));
    auto points2 = resultPoints * flipYMatrix * t;

    // auto t3 = glm::scale(t1, glm::vec2{ 2, 1 });
    auto t3 = glm::scale(glm::mat3{ 1 }, glm::vec2{ 2, 1 });
    auto t4 = t * t3;
    auto _ = 1 + 1;
  }
}

TEST_F(VggResizingTestSuite, DecomposeMatrix)
{
  SKIP_DEBUG_TEST;

  // Given
  {
    // sketch 400x400
    const glm::mat3 mat{ 0.7660444378852844, -0.6427876353263855, 0,
                         0.6427876353263855, 0.7660444378852844,  0,
                         264.303218818517,   -166.59799083442599, 1 };
    glm::vec2       scale;
    float           angle;
    glm::quat       quat;
    glm::vec2       skew;
    glm::vec2       trans;
    glm::vec3       persp;

    decompose(mat, scale, angle, quat, skew, trans, persp);
    auto angle2 = glm::degrees(angle);
    auto _ = 1 + 1;
  }

  {
    // sketch 400x400
    const glm::mat3 mat{ 0.7660444378852844, -0.6427876353263855, 0,
                         0.6427876353263855, 0.7660444378852844,  0,
                         496.4554988077208,  -99.75053319872075,  1 };
    glm::vec2       scale;
    float           angle;
    glm::quat       quat;
    glm::vec2       skew;
    glm::vec2       trans;
    glm::vec3       persp;

    decompose(mat, scale, angle, quat, skew, trans, persp);
    auto angle2 = glm::degrees(angle);
    auto _ = 1 + 1;
  }
  {
    // fig 400x400
    const glm::mat3 mat{ 0.7660444378852844,  0.6427876353263855,  0,
                         -0.6427876353263855, 0.7660444378852844,  0,
                         252.23641967773438,  -327.93597412109375, 1 };
    glm::vec2       scale;
    float           angle;
    glm::quat       quat;
    glm::vec2       skew;
    glm::vec2       trans;
    glm::vec3       persp;

    decompose(mat, scale, angle, quat, skew, trans, persp);
    auto angle2 = glm::degrees(angle);
    auto _ = 1 + 1;
  }

  {
    // fig 800x400
    const glm::mat3 mat{ 0.9221302270889282,  0.3868795931339264,  0,
                         -0.8590516448020935, 0.5118889212608337,  0,
                         504.47283935546875,  -327.93597412109375, 1 };
    glm::vec2       scale;
    float           angle;
    glm::quat       quat;
    glm::vec2       skew;
    glm::vec2       trans;
    glm::vec3       persp;

    decompose(mat, scale, angle, quat, skew, trans, persp);
    auto angle2 = glm::degrees(angle);
    auto _ = 1 + 1;
  }
}

TEST_F(VggResizingTestSuite, ResizeRectangle)
{
  // Given
  setupWithExpanding("testDataDir/resizing/rotated/rectangle/");

  // When
  layoutAllPage(Layout::Size{ 800, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 546.21014404296875, 254.85577392578125 },
                                              { 107.46025085449219, 124.55353546142578 } } };
  EXPECT_TRUE(descendantFrame({ 2 }) == expectedFrames[0]);
}

TEST_F(VggResizingTestSuite, ResizeTriangle)
{
  // Given
  setupWithExpanding("testDataDir/resizing/rotated/triangle/sketch/");

  // When
  layoutAllPage(Layout::Size{ 800, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 496.45550537109375, 99.750534057617188 },
                                              { 170.443115234375, 224.5443115234375 } } };
  EXPECT_TRUE(descendantFrame({ 0 }) == expectedFrames[0]);
}

TEST_F(VggResizingTestSuite, ResizeEllipse)
{
  // Given
  setupWithExpanding("testDataDir/resizing/rotated/ellipse/");

  // When
  layoutAllPage(Layout::Size{ 800, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 576.74481201171875, 133.94818115234375 },
                                              { 238.45028686523438, 163.73127746582031 } } };
  EXPECT_TRUE(descendantFrame({ 0 }) == expectedFrames[0]);
}

TEST_F(VggResizingTestSuite, ResizeTextScale)
{
  // Given
  setupWithExpanding("testDataDir/resizing/rotated/text/");

  // When
  layoutAllPage(Layout::Size{ 800, 400 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 258.9304898734249, -26.859742651458987 },
                                              { 400, 100 } } };
  EXPECT_TRUE(descendantFrame({ 0 }) == expectedFrames[0]);
}

TEST_F(VggResizingTestSuite, ResizeVectorNetwork)
{
  // Given
  setupWithExpanding("testDataDir/resizing/vector_network/");

  // When
  layoutAllPage(Layout::Size{ 800, 800 });

  // Then
  std::vector<Layout::Rect> expectedFrames{ { { 117.78839874267578, 0.0 },
                                              { 34.54272211814413, 78.95296786693817 } } };
  EXPECT_TRUE(descendantFrame({ 1, 0, 0 }) == expectedFrames[0]);
}

TEST_F(VggResizingTestSuite, HandleParentOrigin)
{
  // Given
  setupWithExpanding("testDataDir/resizing/201_handle_parent_origin/");

  // When
  layoutAllPage(Layout::Size{ 1920, 1080 });
  // layoutAllPage(Layout::Size{ 1056, 336 });

  // Then
  {
    std::vector<Layout::Rect> expectedBounds{ { { 0, 0 }, { 1920, 1080 } } };
    auto                      bounds = firstPage()->children()[0]->bounds();
    EXPECT_TRUE(bounds == expectedBounds[0]);
  }
  {
    std::vector<Layout::Rect> expectedFrames{ { { 0, 0 }, { 1920, 1080 } } };
    EXPECT_TRUE(descendantFrame({ 0 }, 0) == expectedFrames[0]);
  }
}

TEST_F(VggResizingTestSuite, MinWidthFixStartEnd)
{
  setupWithExpanding("testDataDir/resizing/101_constraint__fix_start_and_end/");

  layoutAllPage(Layout::Size{ 0.1, 1080 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.width, 1.);
  }

  layoutAllPage(Layout::Size{ 300, 1080 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.width, 200.);
  }
}

TEST_F(VggResizingTestSuite, MinWidthFixStart)
{
  setupWithExpanding("testDataDir/resizing/102_constraint__fix_start_and_scale/");

  layoutAllPage(Layout::Size{ 0.1, 1080 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.width, 1.);
  }

  layoutAllPage(Layout::Size{ 300, 1080 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.width, 200.);
  }
}

TEST_F(VggResizingTestSuite, MinWidthFixEnd)
{
  setupWithExpanding("testDataDir/resizing/103_constraint__fix_end_and_scale/");

  layoutAllPage(Layout::Size{ 0.1, 1080 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.width, 1.);
  }

  layoutAllPage(Layout::Size{ 300, 1080 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.width, 200.);
  }
}

TEST_F(VggResizingTestSuite, MinHeightFixStartEnd)
{
  setupWithExpanding("testDataDir/resizing/111_constraint__fix_top_and_bottom/");

  layoutAllPage(Layout::Size{ 300, 0.1 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.height, 1.);
  }

  layoutAllPage(Layout::Size{ 300, 300 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.height, 200.);
  }
}

TEST_F(VggResizingTestSuite, MinHeightFixStart)
{
  setupWithExpanding("testDataDir/resizing/112_constraint__fix_top_and_scale/");

  layoutAllPage(Layout::Size{ 300, 0.1 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.height, 1.);
  }

  layoutAllPage(Layout::Size{ 300, 300 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.height, 200.);
  }
}

TEST_F(VggResizingTestSuite, MinHeightFixEnd)
{
  setupWithExpanding("testDataDir/resizing/113_constraint__fix_bottom_and_scale/");

  layoutAllPage(Layout::Size{ 300, 0.1 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.height, 1.);
  }

  layoutAllPage(Layout::Size{ 300, 300 });
  {
    auto bounds = firstPage()->children()[0]->bounds();
    EXPECT_DOUBLE_EQ(bounds.size.height, 200.);
  }
}