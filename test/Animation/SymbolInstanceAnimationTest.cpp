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

#include "Adapter/Common/Container.hpp"
#include "Adapter/Environment.hpp"
#include "Application/Presenter.hpp"

#include "test_config.hpp"
#include "container/MockSkiaGraphicsContext.hpp"

#include <gtest/gtest.h>
#include <thread>

namespace VGG::test::animation
{

class SymbolInstanceAnimationTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Container> m_container;

  std::shared_ptr<ISdk> sut()
  {
    return m_container->sdk();
  }

  void SetUp() override
  {
    m_container.reset(new Container);

    std::unique_ptr<layer::SkiaGraphicsContext> graphicsContext{ new MockSkiaGraphicsContext };
    m_container->setGraphicsContext(graphicsContext, 600, 800);
  }
  void TearDown() override
  {
    Environment::tearDown();
  }
};

TEST_F(SymbolInstanceAnimationTestSuite, PresentDismissSetState)
{
  std::string filePath = "testDataDir/Animation/SymbolInstance/";
  auto        result = m_container->load(filePath);
  ASSERT_TRUE(result);
  {
    auto success = sut()->pushFrame("0:12", {});
    ASSERT_TRUE(success);
  }

  ISdk::StateOptions opts;
  opts.animation.type = "dissolve";
  {
    auto success = sut()->presentState("0:13__0:5", "0:5", "0:7", opts);
    EXPECT_TRUE(success);
  }
  {
    auto success = sut()->dismissState("0:13__0:5");
    EXPECT_TRUE(success);
  }
  {
    auto success = sut()->setState("0:13__0:5", "0:5", "0:7", opts);
    EXPECT_TRUE(success);
  }
}

} // namespace VGG::test::animation
