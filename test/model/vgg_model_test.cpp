#include "Domain/Daruma.hpp"

#include "Application/UIEvent.hpp"
#include "Domain/RawJsonDocument.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

using namespace VGG;

const auto g_eventNameClick = uiEventTypeToString(EUIEventType::CLICK);

class VggModelTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<Daruma> m_sut;

  void SetUp() override
  {
    makeNormalSut();
  }

  void TearDown() override
  {
    m_sut.reset();
  }

  void makeNormalSut()
  {
    m_sut.reset(new Daruma(Helper::RawJsonDocumentBuilder, Helper::RawJsonDocumentBuilder));
  }
};

TEST_F(VggModelTestSuite, Smoke)
{
  GTEST_SUCCEED();
}

TEST_F(VggModelTestSuite, Load_from_file)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";

  // When
  auto ret = m_sut->load(filePath);

  // Then
  EXPECT_EQ(ret, true);
}

TEST_F(VggModelTestSuite, Load_from_buffer)
{
  // Given
  std::ifstream   file("testDataDir/vgg-daruma.zip", std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size))
  {
    GTEST_FAIL();
  }

  // When
  auto ret = m_sut->load(buffer);

  // Then
  EXPECT_EQ(ret, true);
}

TEST_F(VggModelTestSuite, Get_design_doc)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  // When
  auto& retDoc = m_sut->designDoc();

  // Then
  EXPECT_TRUE(retDoc->content().is_object());
}

TEST_F(VggModelTestSuite, Get_code)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  // When
  auto retCode =
    m_sut->getEventListeners("/artboard/layers/0/childObjects")[g_eventNameClick].front();

  // Then
  EXPECT_TRUE(!retCode.empty());
}

TEST_F(VggModelTestSuite, add_event_listener)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  m_sut->removeEventListener("/fake", g_eventNameClick, "console.log('hello');");
  auto eventListenersBefore = m_sut->getEventListeners("/fake");

  // When
  m_sut->addEventListener("/fake", g_eventNameClick, "console.log('hello');");

  // Then
  auto eventListenersAfter = m_sut->getEventListeners("/fake");
  EXPECT_EQ(eventListenersBefore.size() + 1, eventListenersAfter.size());
}

TEST_F(VggModelTestSuite, remove_event_listener)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  m_sut->removeEventListener("/fake", g_eventNameClick, "console.log('hello');");
  m_sut->addEventListener("/fake", g_eventNameClick, "console.log('hello');");
  auto eventListenersBefore = m_sut->getEventListeners("/fake")[g_eventNameClick];

  // When
  m_sut->removeEventListener("/fake", g_eventNameClick, "console.log('hello');");

  // Then
  auto eventListenersAfter = m_sut->getEventListeners("/fake")[g_eventNameClick];
  EXPECT_EQ(eventListenersBefore.size() - 1, eventListenersAfter.size());
}

TEST_F(VggModelTestSuite, get_event_listeners)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  m_sut->removeEventListener("/fake", g_eventNameClick, "console.log('hello');");

  auto eventListenersBefore = m_sut->getEventListeners("/fake");
  m_sut->addEventListener("/fake", g_eventNameClick, "console.log('hello');");

  // When
  auto eventListenersAfter = m_sut->getEventListeners("/fake");

  // Then
  EXPECT_EQ(eventListenersBefore.size() + 1, eventListenersAfter.size());
}

TEST_F(VggModelTestSuite, load_layout_doc)
{
  // Given
  std::string filePath = "testDataDir/layout/0_space_between/";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  // When
  auto layoutDoc = m_sut->layoutDoc();

  // Then
  EXPECT_TRUE(layoutDoc);
}

TEST_F(VggModelTestSuite, GetTexts)
{
  // Given
  std::string filePath = "testDataDir/vgg-daruma.zip";
  auto        ret = m_sut->load(filePath);
  EXPECT_EQ(ret, true);

  // When
  auto retTexts = m_sut->texts();

  // Then
  EXPECT_EQ(retTexts.size(), 1);
}
