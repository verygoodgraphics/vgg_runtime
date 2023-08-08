#include "Domain/Layout/Layout.hpp"

#include "model/daruma_helper.hpp"

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
  m_sut.reset(new Layout::Layout(nullptr));
  EXPECT_TRUE(m_sut);
}

TEST_F(VggLayoutTestSuite, normalize_point)
{
  // Given
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder) };
  std::string file_path = "testDataDir/point/gradient";
  daruma->load(file_path);

  m_sut.reset(new Layout::Layout(daruma));

  // When
  auto normalized_design_json = m_sut->normalizePoint();

  // Then
  nlohmann::json::json_pointer point_path{
    "/artboard/0/layers/0/childObjects/0/shape/subshapes/0/subGeometry/points/0/point"
  };
  auto point = normalized_design_json[point_path];
  auto x = point[0].get<float>();
  auto y = point[1].get<float>();

  // -1 <= x <= 1
  EXPECT_GE(x, -1);
  EXPECT_LE(x, 1);

  // -1 <= y <= 1
  EXPECT_GE(y, -1);
  EXPECT_LE(y, 1);
}