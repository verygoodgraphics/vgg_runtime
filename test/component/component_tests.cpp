/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MockSkiaGraphicsContext.hpp"

#include "Entry/Component/Component.hpp"

#include "test_config.hpp"

#include <gtest/gtest.h>

#include <thread>

using namespace VGG;
class ComponentTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Component> m_sut;

  void SetUp() override
  {
    m_sut.reset(new Component);
  }
};

TEST_F(ComponentTestSuite, Smoke)
{
  // Given

  // When

  // Then
}

TEST_F(ComponentTestSuite, LoadModel)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma-2";

  // When
  auto result = m_sut->load(filePath);

  // Then
  EXPECT_TRUE(result);
}

TEST_F(ComponentTestSuite, Render)
{
  // Given
  std::unique_ptr<layer::SkiaGraphicsContext> graphicsContext{ new MockSkiaGraphicsContext };
  m_sut->setGraphicsContext(graphicsContext, 600, 800);

  std::string filePath = "testDataDir/vgg-daruma-2";
  auto        result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  // When
  {
    auto ret = m_sut->run();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // makeFrame
    ret = m_sut->run();
    EXPECT_TRUE(ret);
  }

  // Then
}