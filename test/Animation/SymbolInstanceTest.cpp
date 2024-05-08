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

#include "Adapter/Environment.hpp"
#include "Application/Presenter.hpp"
#include "Entry/Container/Container.hpp"

#include "test_config.hpp"

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
  }
  void TearDown() override
  {
    Environment::tearDown();
  }
};

TEST_F(SymbolInstanceAnimationTestSuite, Smoke)
{
  std::string filePath =
    "/Users/houguanhua/code/vgg/vgg_runtime_C/_feature/animation/symbol_instance/swap_master/";
  auto result = m_container->load(filePath);
  ASSERT_TRUE(result);
  {
    auto success = sut()->setCurrentFrameById("0:12");
    ASSERT_TRUE(success);
  }

  {
    ISdk::StateOptions opts;
    auto               success = sut()->setState("0:13__0:8", "0:8", "0:10", opts);
    EXPECT_FALSE(success);
  }
}

} // namespace VGG::test::animation