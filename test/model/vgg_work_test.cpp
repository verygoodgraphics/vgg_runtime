#include "VggWork.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

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

TEST_F(VggWorkTestSuite, Load_from_buffer)
{
  // Given
  std::ifstream file("testDataDir/vgg-work.zip", std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<unsigned char> buffer(size);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
  {
    GTEST_FAIL();
  }

  // When
  auto ret = m_sut.load(buffer);

  // Then
  EXPECT_EQ(ret, true);
}