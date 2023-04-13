#include "VggWork.hpp"

#include <gtest/gtest.h>

class VggWorkTestSuite : public ::testing::Test
{
protected:
  VggWork m_sut;

  void SetUp() override
  {
  }

  void TearDown() override
  {
  }
};

TEST_F(VggWorkTestSuite, Smoke)
{
  GTEST_SUCCEED();
}

TEST_F(VggWorkTestSuite, Load_from_file)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";

  // When
  auto ret = m_sut.load(file_path);

  // Then
  EXPECT_EQ(ret, true);
}