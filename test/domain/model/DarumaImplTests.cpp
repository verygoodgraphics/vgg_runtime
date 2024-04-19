/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "test_config.hpp"

#include "Domain/Model/DarumaImpl.hpp"

using namespace VGG::Model::Detail;

class DarumaImplTestSuite : public ::testing::Test
{
protected:
  std::unique_ptr<DarumaImpl> m_sut;

  void SetUp() override
  {
    m_sut.reset(new DarumaImpl);
  }
  void TearDown() override
  {
  }
};

TEST_F(DarumaImplTestSuite, GetBreakpoints)
{
  // Given
  const auto settingsJson = R"(
{
  "launchFrameId": "-",
  "currentTheme": "dark",
  "themes": [
    "dark",
    "light"
  ],
  "breakpoints": [
    {
      "minWidth": 0,
      "dark": "dark:320",
      "light": "light:320"
    },
    {
      "minWidth": 768,
      "dark": "dark:768",
      "light": "light:768"
    }
  ]
}
)";
  m_sut->setSettings(nlohmann::json::parse(settingsJson));

  constexpr auto dark320 = "dark:320";
  constexpr auto dark768 = "dark:768";
  constexpr auto light320 = "light:320";
  constexpr auto light768 = "light:768";

  // dark theme
  {
    {
      auto theme = m_sut->currentTheme();
      EXPECT_EQ(theme, "dark");
    }
    m_sut->setCurrentTheme("xxx");
    {
      auto theme = m_sut->currentTheme();
      EXPECT_EQ(theme, "dark");
    }
  }

  {
    auto id = m_sut->frameIdForWidth(0);
    EXPECT_EQ(id, dark320);
  }
  {
    auto id = m_sut->frameIdForWidth(319);
    EXPECT_EQ(id, dark320);
  }
  {
    auto id = m_sut->frameIdForWidth(320);
    EXPECT_EQ(id, dark320);
  }
  {
    auto id = m_sut->frameIdForWidth(321);
    EXPECT_EQ(id, dark320);
  }
  {
    auto id = m_sut->frameIdForWidth(767);
    EXPECT_EQ(id, dark320);
  }
  {
    auto id = m_sut->frameIdForWidth(768);
    EXPECT_EQ(id, dark768);
  }
  {
    auto id = m_sut->frameIdForWidth(800);
    EXPECT_EQ(id, dark768);
  }

  // light theme
  {
    auto light = "light";
    {
      auto theme = m_sut->currentTheme();
      EXPECT_NE(theme, light);
    }
    m_sut->setCurrentTheme(light);
    {
      auto theme = m_sut->currentTheme();
      EXPECT_EQ(theme, light);
    }
  }
  {
    auto id = m_sut->frameIdForWidth(0);
    EXPECT_EQ(id, light320);
  }
  {
    auto id = m_sut->frameIdForWidth(319);
    EXPECT_EQ(id, light320);
  }
  {
    auto id = m_sut->frameIdForWidth(320);
    EXPECT_EQ(id, light320);
  }
  {
    auto id = m_sut->frameIdForWidth(321);
    EXPECT_EQ(id, light320);
  }
  {
    auto id = m_sut->frameIdForWidth(767);
    EXPECT_EQ(id, light320);
  }
  {
    auto id = m_sut->frameIdForWidth(768);
    EXPECT_EQ(id, light768);
  }
  {
    auto id = m_sut->frameIdForWidth(800);
    EXPECT_EQ(id, light768);
  }

  // Then
}

TEST_F(DarumaImplTestSuite, EmptySettings)
{
  // Given
  const auto settingsJson = R"( {} )";
  m_sut->setSettings(nlohmann::json::parse(settingsJson));

  {
    auto id = m_sut->frameIdForWidth(0);
    EXPECT_TRUE(id.empty());
  }
  {
    auto id = m_sut->frameIdForWidth(319);
    EXPECT_TRUE(id.empty());
  }
}

TEST_F(DarumaImplTestSuite, OnlyOneBreakpoint)
{
  // Given
  const auto settingsJson = R"(
{
  "launchFrameId": "-",
  "currentTheme": "dark",
  "themes": [
    "dark",
    "light"
  ],
  "breakpoints": [
    {
      "minWidth": 768,
      "dark": "dark:768",
      "light": "light:768"
    }
  ]
}
)";
  m_sut->setSettings(nlohmann::json::parse(settingsJson));
  constexpr auto dark768 = "dark:768";

  {
    auto id = m_sut->frameIdForWidth(767);
    EXPECT_EQ(id, dark768);
  }
  {
    auto id = m_sut->frameIdForWidth(768);
    EXPECT_EQ(id, dark768);
  }
  {
    auto id = m_sut->frameIdForWidth(800);
    EXPECT_EQ(id, dark768);
  }
}