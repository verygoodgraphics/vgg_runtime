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
  std::shared_ptr<Daruma> daruma{ new Daruma(Helper::RawJsonDocumentBuilder) };
  std::string file_path = "testDataDir/vgg-daruma";
  daruma->load(file_path);

  // When
  m_sut.reset(new Layout::Layout(daruma));

  // Then
  EXPECT_TRUE(m_sut);
}
