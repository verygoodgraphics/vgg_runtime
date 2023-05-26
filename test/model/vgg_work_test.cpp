#include "Model/VggWork.hpp"

#include "Presenter/UIEvent.hpp"
#include "Model/RawJsonDocument.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

using namespace VGG;

const auto event_name_click = UIEventTypeToString(UIEventType::click);

class VggWorkTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<VggWork> m_sut;

  void SetUp() override
  {
    make_normal_sut();
  }

  void TearDown() override
  {
    m_sut.reset();
  }

  void make_normal_sut()
  {
    auto fn = [](const json& design_json)
    {
      auto raw_json_doc = new RawJsonDocument();
      raw_json_doc->setContent(design_json);
      return JsonDocumentPtr(raw_json_doc);
    };
    m_sut.reset(new VggWork(fn));
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
  auto ret = m_sut->load(file_path);

  // Then
  EXPECT_EQ(ret, true);
}

TEST_F(VggWorkTestSuite, Load_from_buffer)
{
  // Given
  std::ifstream file("testDataDir/vgg-work.zip", std::ios::binary | std::ios::ate);
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

TEST_F(VggWorkTestSuite, Get_design_doc)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->load(file_path);
  EXPECT_EQ(ret, true);

  // When
  auto& ret_doc = m_sut->designDoc();

  // Then
  EXPECT_TRUE(ret_doc->content().is_object());
}

TEST_F(VggWorkTestSuite, Get_code)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->load(file_path);
  EXPECT_EQ(ret, true);

  // When
  auto ret_code =
    m_sut->getEventListeners("/artboard/layers/0/childObjects")[event_name_click].front();

  // Then
  EXPECT_TRUE(!ret_code.empty());
}

TEST_F(VggWorkTestSuite, add_event_listener)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->load(file_path);
  EXPECT_EQ(ret, true);

  m_sut->removeEventListener("/fake", event_name_click, "console.log('hello');");
  auto event_listeners_before = m_sut->getEventListeners("/fake");

  // When
  m_sut->addEventListener("/fake", event_name_click, "console.log('hello');");

  // Then
  auto event_listeners_after = m_sut->getEventListeners("/fake");
  EXPECT_EQ(event_listeners_before.size() + 1, event_listeners_after.size());
}

TEST_F(VggWorkTestSuite, remove_event_listener)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->load(file_path);
  EXPECT_EQ(ret, true);

  m_sut->removeEventListener("/fake", event_name_click, "console.log('hello');");
  m_sut->addEventListener("/fake", event_name_click, "console.log('hello');");
  auto event_listeners_before = m_sut->getEventListeners("/fake")[event_name_click];

  // When
  m_sut->removeEventListener("/fake", event_name_click, "console.log('hello');");

  // Then
  auto event_listeners_after = m_sut->getEventListeners("/fake")[event_name_click];
  EXPECT_EQ(event_listeners_before.size() - 1, event_listeners_after.size());
}

TEST_F(VggWorkTestSuite, get_event_listeners)
{
  // Given
  std::string file_path = "testDataDir/vgg-work.zip";
  auto ret = m_sut->load(file_path);
  EXPECT_EQ(ret, true);

  m_sut->removeEventListener("/fake", event_name_click, "console.log('hello');");

  auto event_listeners_before = m_sut->getEventListeners("/fake");
  m_sut->addEventListener("/fake", event_name_click, "console.log('hello');");

  // When
  auto event_listeners_after = m_sut->getEventListeners("/fake");

  // Then
  EXPECT_EQ(event_listeners_before.size() + 1, event_listeners_after.size());
}