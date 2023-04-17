#include "Controller.hpp"
#include "MainComposer.hpp"
#include "VggDepContainer.hpp"
#include "VggWork.hpp"

#include <gtest/gtest.h>

class ControllerTestSuite : public ::testing::Test
{
protected:
  Controller m_sut;
  MainComposer composer;

  void SetUp() override
  {
    composer.setup();
  }
  void TearDown() override
  {
    composer.teardown();
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

TEST_F(ControllerTestSuite, OnClick)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut.start(file_path);
  EXPECT_TRUE(ret);

  // When
  m_sut.onClick("/artboard/layers/childObjects");
  // m_sut.onClick("/artboard/layers");

  // Then
}