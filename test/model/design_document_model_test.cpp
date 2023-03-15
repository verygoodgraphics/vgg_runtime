#include "DesignDocumentModel.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <fstream>

using namespace nlohmann;
using VGG::DesignDocumentModel; // https://app.quicktype.io/

#define JSON_SCHEMA_FILE_NAME "./asset/vgg-format.json"
#define DOCUMENT_FILE_NAME "./testDataDir/1_out_json.json"

class DesignDocumentDTOTestSuite : public ::testing::Test
{
protected:
  DesignDocumentModel m_cppData;
  json m_jsonData;

  void SetUp() override
  {
    m_jsonData = load_json(DOCUMENT_FILE_NAME);
    m_cppData = m_jsonData;
  }

  void TearDown() override
  {
  }

  json load_json(const std::string& json_file_name)
  {
    std::ifstream json_fs(json_file_name);
    json json_data = json::parse(json_fs);

    return json_data;
  }

  void read_write_field_by_cpp_object(int read_times, int write_times)
  {
    for (auto i = 0; i < read_times; ++i)
    {
      double v = (*m_cppData.artboard[0].layers[0].child_objects[0].style.borders[0].color).alpha;
    }
    for (auto i = 0; i < write_times; ++i)
    {
      (*m_cppData.artboard[0].layers[0].child_objects[0].style.borders[0].color).alpha = 0.1;
    }
  }

  void read_write_field_by_json_object(int read_times, int write_times)
  {
    for (auto i = 0; i < read_times; ++i)
    {
      double v = m_jsonData["artboard"][0]["layers"][0]["childObjects"][0]["style"]["borders"][0]
                           ["color"]["alpha"];
    }
    for (auto i = 0; i < write_times; ++i)
    {
      m_jsonData["artboard"][0]["layers"][0]["childObjects"][0]["style"]["borders"][0]["color"]
                ["alpha"] = 0.1;
    }
  }
};

TEST_F(DesignDocumentDTOTestSuite, Smoke)
{
  DesignDocumentModel data = load_json(DOCUMENT_FILE_NAME);
  EXPECT_TRUE(!data.artboard.empty());
}

TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_0_0)
{
  read_write_field_by_cpp_object(0, 0);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_8_0)
{
  read_write_field_by_cpp_object(8, 0);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_0_2)
{
  read_write_field_by_cpp_object(0, 2);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_8_2)
{
  read_write_field_by_cpp_object(8, 2);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_80000_0)
{
  read_write_field_by_cpp_object(80000, 0);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_0_20000)
{
  read_write_field_by_cpp_object(0, 20000);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_cpp_object_80000_20000)
{
  read_write_field_by_cpp_object(80000, 20000);
}

TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_0_0)
{
  read_write_field_by_json_object(0, 0);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_8_0)
{
  read_write_field_by_json_object(8, 0);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_0_2)
{
  read_write_field_by_json_object(0, 2);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_8_2)
{
  read_write_field_by_json_object(8, 2);
}

TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_80000_0)
{
  read_write_field_by_json_object(80000, 0);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_0_20000)
{
  read_write_field_by_json_object(0, 20000);
}
TEST_F(DesignDocumentDTOTestSuite, Read_write_field_by_json_object_80000_20000)
{
  read_write_field_by_json_object(80000, 20000);
}
