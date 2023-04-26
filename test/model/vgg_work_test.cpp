#include "Model/VggWork.hpp"

#include "Model/RawJsonDocument.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <vector>

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
  auto ret_code = m_sut->getCode("/artboard/layers/0/childObjects");

  // Then
  EXPECT_TRUE(!ret_code.empty());
}