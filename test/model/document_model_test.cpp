#include "DocumentModel.hpp"

#include <gtest/gtest.h>
#include <memory>

using namespace nlohmann;

#define JSON_SCHEMA_FILE_NAME "./asset/vgg-format.json"
#define DOCUMENT_FILE_NAME "./testDataDir/2020.json"

class DocumentModelTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<DocumentModel> sut;

  void SetUp() override
  {
    std::ifstream schema_fs(JSON_SCHEMA_FILE_NAME);
    json schema = json::parse(schema_fs);

    std::ifstream document_fs(DOCUMENT_FILE_NAME);
    json document = json::parse(document_fs);

    sut.reset(new DocumentModel(schema, document));
  }

  void TearDown() override
  {
  }
};

TEST_F(DocumentModelTestSuite, Smoke)
{
  EXPECT_TRUE(sut);
}

TEST_F(DocumentModelTestSuite, EditColor)
{
  const auto path = "/artboard/0/layers/0/childObjects/0/style/borders/0/color/alpha"_json_pointer;
  const auto value = 0.1;
  sut->replaceAt(path, value);

  const auto document = sut->documentJson();
  const auto new_value = document[path];

  EXPECT_EQ(new_value, value);
}
