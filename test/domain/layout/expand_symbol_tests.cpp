#include "Domain/Layout/ExpandSymbol.hpp"

#include "Domain/Layout/Helper.hpp"
#include "Domain/Model/JsonKeys.hpp"
#include "VggFloat.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

using namespace VGG;
using namespace VGG::Layout;

class VggExpandSymbolTestSuite : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }

  void TearDown() override
  {
  }

  void write_result_json(const nlohmann::json& json)
  {
    std::string out_file_path = "tmp/expanded_design.json";
    Helper::write_json(json, out_file_path);
  }
};

TEST_F(VggExpandSymbolTestSuite, Smoke)
{
  // Given
  std::string file_path = "testDataDir/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  EXPECT_TRUE(result_json.is_object());
}

TEST_F(VggExpandSymbolTestSuite, fill_childObjects)
{
  // Given
  std::string file_path = "testDataDir/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/childObjects" };
  auto instance_child = result_json[path];
  EXPECT_TRUE(instance_child.is_array());
}

TEST_F(VggExpandSymbolTestSuite, scale)
{ // Given
  std::string file_path = "testDataDir/symbol/scale/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();
  write_result_json(result_json);

  // Then
  nlohmann::json::json_pointer path{ "/frames/1/childObjects/0/childObjects/1" };
  auto child = result_json[path];
  auto child_bounds = child[k_bounds].get<Rect>();
  auto child_frame = child[k_frame].get<Rect>();

  Rect expect_bounds{ 0, 0, 260.800018, 133.333344 };
  Rect expect_frame{ 57.5999985, -200, 260.800018, 133.333344 };

  EXPECT_EQ(child_bounds, expect_bounds);
  EXPECT_EQ(child_frame, expect_frame);
}

TEST_F(VggExpandSymbolTestSuite, expand_masterId_overridden_instance)
{
  // Given
  std::string file_path = "testDataDir/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/5/childObjects/1" };
  auto instance_child = result_json[path];
  EXPECT_EQ(instance_child[k_master_id], "87621494-BE66-4837-9F83-EE3D6390C1D6");
  EXPECT_TRUE(instance_child[k_override_values].is_array());
  EXPECT_TRUE(instance_child[k_override_values].empty());
}
// instance of one master containing instance overide width & height