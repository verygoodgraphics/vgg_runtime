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
#include "MockSkiaGraphicsContext.hpp"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>
#include <thread>

using namespace VGG;
class ContainerTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Container> m_sut;

  void SetUp() override
  {
    m_sut.reset(new Container);
  }
  void TearDown() override
  {
    Environment::tearDown();
  }
};

TEST_F(ContainerTestSuite, Smoke)
{
  // Given

  // When

  // Then
}

TEST_F(ContainerTestSuite, LoadModel)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma-2";

  // When
  auto result = m_sut->load(filePath);

  // Then
  EXPECT_TRUE(result);
}

TEST_F(ContainerTestSuite, Configure)
{
  auto sdk = m_sut->sdk();

  // configure before loading vgg file
  EXPECT_NO_THROW(sdk->setFitToViewportEnabled(true));
  EXPECT_NO_THROW(sdk->setBackgroundColor(0xFFFF0000));

  std::string filePath = "testDataDir/frame_list/";
  auto        result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  // configure after loading vgg file
  EXPECT_NO_THROW(sdk->setFitToViewportEnabled(false));
  EXPECT_NO_THROW(sdk->setFitToViewportEnabled(true));

  EXPECT_NO_THROW(sdk->setBackgroundColor(0xFFFF0000));
}

TEST_F(ContainerTestSuite, Render)
{
  SKIP_DEBUG_TEST;

  // Given
  std::unique_ptr<layer::SkiaGraphicsContext> graphicsContext{ new MockSkiaGraphicsContext };
  m_sut->setGraphicsContext(graphicsContext, 600, 800);

  std::string filePath = "testDataDir/vgg-daruma-2";
  auto        result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  // When
  {
    auto ret = m_sut->paint();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // makeFrame
    ret = m_sut->paint();
    EXPECT_TRUE(ret);
  }

  // Then
}

TEST_F(ContainerTestSuite, HandleEvent)
{
  // Given
  std::unique_ptr<layer::SkiaGraphicsContext> graphicsContext{ new MockSkiaGraphicsContext };
  m_sut->setGraphicsContext(graphicsContext, 600, 800);

  std::string filePath = "testDataDir/vgg-daruma-2";
  auto        result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  bool called{ false };
  m_sut->setEventListener([&called](std::string type, std::string id, std::string path)
                          { called = true; });

  // When
  UEvent evt;
  evt.touch.type = VGG_TOUCHDOWN;
  evt.touch.windowX = 100;
  evt.touch.windowY = 100;

  m_sut->onEvent(evt);

  // Then
  EXPECT_TRUE(called);
}

TEST_F(ContainerTestSuite, SetBackgrondColor)
{
  // Given
  std::unique_ptr<layer::SkiaGraphicsContext> graphicsContext{ new MockSkiaGraphicsContext };
  m_sut->setGraphicsContext(graphicsContext, 600, 800);

  std::string filePath = "testDataDir/vgg-daruma-2";
  auto        result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  // Then
  auto sdk = m_sut->sdk();
  EXPECT_NO_THROW(sdk->setBackgroundColor(0xFFFF0000));
}

TEST_F(ContainerTestSuite, Sdk)
{
  std::string filePath = "testDataDir/frame_list/";

  auto result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  auto sdk = m_sut->sdk();
  {
    const auto& framesJsonString = sdk->getFramesInfo();

    auto frames = nlohmann::json::parse(framesJsonString);
    EXPECT_EQ(frames.size(), 3);
  }

  {
    const auto id = sdk->currentFrameId();
    EXPECT_EQ(id, "1:2");
  }

  {
    auto id1 = sdk->currentFrameId();
    auto success = sdk->setCurrentFrameById("#invalidFrameId");
    EXPECT_FALSE(success);
    auto id2 = sdk->currentFrameId();
    EXPECT_EQ(id1, id2);
  }
  {
    const std::string newId{ "1:3" };

    auto id1 = sdk->currentFrameId();
    EXPECT_TRUE(id1 != newId);

    auto success = sdk->setCurrentFrameById(newId);
    EXPECT_TRUE(success);

    auto id2 = sdk->currentFrameId();
    EXPECT_TRUE(id2 == newId);
  }

  {
    const auto launchFrameId = sdk->launchFrameId();
    EXPECT_TRUE(launchFrameId == "");
  }
  {
    auto success = sdk->setLaunchFrameById("#invalidFrameName");
    EXPECT_FALSE(success);
  }
  {
    const std::string newId{ "1:2" };

    const auto id1 = sdk->launchFrameId();
    EXPECT_TRUE(id1 != newId);

    auto success = sdk->setLaunchFrameById(newId);
    EXPECT_TRUE(success);

    const auto id2 = sdk->launchFrameId();
    EXPECT_TRUE(id2 == newId);
  }

  {
    const auto& fontsJsonString = sdk->requiredFonts();

    auto fonts = nlohmann::json::parse(fontsJsonString);
    EXPECT_EQ(fonts.size(), 1);

    FontInfo font = fonts[0];
    EXPECT_EQ(font.familyName, "Inter");
    EXPECT_EQ(font.subfamilyName, "Regular");
  }

  {
    auto buffer = sdk->vggFileBuffer();
    EXPECT_EQ(buffer.size(), 1332);
  }

  {
    auto retTexts = sdk->texts();
    EXPECT_EQ(retTexts.size(), 3);
  }
}

TEST_F(ContainerTestSuite, SdkElementApis)
{
  std::string filePath = "../../examples/counter/";

  auto result = m_sut->load(filePath);
  EXPECT_TRUE(result);

  auto sdk = m_sut->sdk();

  {
    auto buttonId = "#counterButton";
    auto s = sdk->getElement(buttonId);
    auto j = nlohmann::json::parse(s);
    EXPECT_NEAR(j["style"]["fills"][0]["color"]["alpha"], 1, EPSILON);

    j["style"]["fills"][0]["color"]["alpha"] = 0.5;
    sdk->updateElement(buttonId, j.dump());
    s = sdk->getElement(buttonId);
    j = nlohmann::json::parse(s);
    EXPECT_EQ(j["style"]["fills"][0]["color"]["alpha"], 0.5);
  }

  {
    auto countId = "#count";
    auto s = sdk->getElement(countId);
    auto j = nlohmann::json::parse(s);
    EXPECT_EQ(j["content"], "0");

    j = nlohmann::json::object(); // Only update patches
    j["content"] = "100";
    sdk->updateElement(countId, j.dump());
    s = sdk->getElement(countId);
    j = nlohmann::json::parse(s);
    EXPECT_EQ(j["content"], "100");
    EXPECT_EQ(j["name"], countId); // The original field should not be changed
  }
}