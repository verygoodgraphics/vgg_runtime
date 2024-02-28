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

#include "Utility/VggTimer.hpp"

#include "Application/RunLoop.hpp"
#include "Utility/VggDate.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <cstdio>

using namespace VGG;

class VggTimerTestSuite : public ::testing::Test
{
protected:
};

TEST_F(VggTimerTestSuite, Smoke)
{
  EXPECT_NO_THROW((Timer{ 0, []() {} }));
}

TEST_F(VggTimerTestSuite, OnceTimer)
{
  auto runLoop = RunLoop::sharedInstance();

  auto intervalSeconds = 0.1;
  auto t1 = nowTimestampInSeconds();

  auto  count = 0;
  Timer timer{ intervalSeconds, [&count]() { count++; } };

  while (!runLoop->empty())
  {
    runLoop->dispatch();
  }

  auto t2 = nowTimestampInSeconds();
  EXPECT_EQ(count, 1);
  EXPECT_NEAR(t2 - t1, intervalSeconds, 0.1);
}

TEST_F(VggTimerTestSuite, CancelOnceTimer)
{
  auto runLoop = RunLoop::sharedInstance();

  auto intervalSeconds = 0.1;

  auto  count = 0;
  Timer timer{ intervalSeconds, [&count]() { count++; } };
  timer.invalidate();

  while (!runLoop->empty())
  {
    runLoop->dispatch();
  }

  EXPECT_EQ(count, 0);
}

TEST_F(VggTimerTestSuite, RepeatedTimer)
{
  auto runLoop = RunLoop::sharedInstance();

  auto intervalSeconds = 0.1;

  auto  count = 0;
  Timer timer{ intervalSeconds, [&count]() { count++; }, true };

  const auto maxCount = 10;

  while (!runLoop->empty())
  {
    runLoop->dispatch();
    if (count >= maxCount)
    {
      timer.invalidate();
    }
  }

  EXPECT_EQ(count, maxCount);
}