#include "daruma_helper.hpp"
#include "test_config.hpp"

#include "Domain/Model/DesignModel.hpp"

#include <gtest/gtest.h>

using namespace VGG::Model;

class DesignModelTestSuite : public ::testing::Test
{
};

TEST_F(DesignModelTestSuite, Smoke)
{
  SKIP_LOCAL_TEST;

  std::string filePath = "testDataDir/_local/vgg/design.json";
  auto        designJson = Helper::load_json(filePath);

  DesignModel data = designJson;
  EXPECT_EQ(data.frames.size(), 1);

  nlohmann::json json = data;
  EXPECT_EQ(json["frames"].size(), 1);
}

TEST_F(DesignModelTestSuite, SymbolInstance)
{
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto        designJson = Helper::load_json(filePath);

  DesignModel data = designJson;
  EXPECT_EQ(data.frames.size(), 2);

  nlohmann::json json = data;
  EXPECT_EQ(json["frames"].size(), 2);
}