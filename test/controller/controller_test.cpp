#include "Controller.hpp"
#include "VggDepContainer.hpp"
#include "VggWork.hpp"

#include <gtest/gtest.h>

class ControllerTestSuite : public ::testing::Test
{
protected:
  Controller m_sut;

  void SetUp() override
  {
  }
  void TearDown() override
  {
  }
};

TEST_F(ControllerTestSuite, Smoke)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";

  // When
  auto ret = m_sut.start(file_path);

  // Then
  EXPECT_TRUE(ret);
  // Then
  auto vgg_work = VggDepContainer<std::shared_ptr<VggWork>>::get();
  EXPECT_TRUE(vgg_work);
}