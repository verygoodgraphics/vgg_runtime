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

  void debug_write_result_json(const nlohmann::json& json)
  {
    std::string out_file_path = "tmp/design.json";
    Helper::write_json(json, out_file_path);
  }
};

TEST_F(VggExpandSymbolTestSuite, Smoke)
{
  // Given
  std::string file_path = "testDataDir/symbol/symbol_instance/design.json";
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
  std::string file_path = "testDataDir/symbol/symbol_instance/design.json";
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

  // Then
  nlohmann::json::json_pointer path{ "/frames/1/childObjects/0/childObjects/1" };
  auto child = result_json[path];
  auto child_bounds = child[k_bounds].get<Rect>();
  auto child_frame = child[k_frame].get<Rect>();
  auto child_matrix = child[k_matrix].get<Matrix>();

  Rect expect_bounds{ 0, 0, 637.656005, 129.333344 };
  Rect expect_frame{ 140.832001, -194, 637.656005, 129.333344 };
  Matrix expect_matrix{ 1, 0, 0, 1, 140.832001, -194 };

  EXPECT_EQ(child_bounds, expect_bounds);
  EXPECT_EQ(child_frame, expect_frame);
  EXPECT_EQ(child_matrix, expect_matrix);

  // Then
  nlohmann::json::json_pointer path2{
    "/frames/1/childObjects/0/childObjects/2/childObjects/0/shape/subshapes/0/subGeometry/points/2"
  };
  auto point = result_json[path2][k_point].get<Point>();
  Point expect_point{ 704.1600341796875, -129.33334350585938 };
  EXPECT_EQ(point, expect_point);
}

TEST_F(VggExpandSymbolTestSuite, expand_masterId_overridden_instance)
{
  // Given
  std::string file_path = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/4/childObjects/1" };
  auto instance_child = result_json[path];
  EXPECT_TRUE(instance_child[k_master_id].is_null());
  EXPECT_TRUE(instance_child[k_override_values].is_null());
}

TEST_F(VggExpandSymbolTestSuite, color_override)
{ // Given
  std::string file_path = "testDataDir/symbol/scale/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{
    "/frames/1/childObjects/0/childObjects/2/childObjects/0/style/fills/0/color/blue"
  };
  double blue = result_json[path];

  EXPECT_DOUBLE_EQ(blue, 0.7517530913433672);
}

TEST_F(VggExpandSymbolTestSuite, override_with_star_wildcard)
{
  // Given
  std::string file_path = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{
    "/frames/0/childObjects/2/childObjects/0/attr/0/fills/0/color/blue"
  };
  double blue = result_json[path];

  EXPECT_DOUBLE_EQ(blue, 0.01908801696712621);
}

TEST_F(VggExpandSymbolTestSuite, override_master_own_style)
{
  // Given
  std::string file_path = "testDataDir/symbol/instance_override_master‘s_own_style/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/style/fills" };
  auto fills = result_json[path].dump();

  EXPECT_EQ(fills, "[]");
}

TEST_F(VggExpandSymbolTestSuite, override_duplicated_object_id)
{
  // Given
  std::string file_path = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{
    "/frames/0/childObjects/4/childObjects/1/childObjects/0/content"
  };
  auto content = result_json[path].get<std::string>();

  EXPECT_EQ(content, "symbol-change");
}

TEST_F(VggExpandSymbolTestSuite, unique_object_id_in_instance_tree)
{
  // Given
  std::string file_path = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(file_path);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/4/childObjects/1/childObjects/0/id" };
  auto id = result_json[path].get<std::string>();

  EXPECT_EQ(id,
            "98C9A450-A48C-4072-B310-E9E80A20F309__651675C7-452D-48D3-A84A-A6CF6796E3B3__95C02DB7-"
            "5EC9-4A61-97B1-0FFBE0C30E83");
}

// todo, validate expanded json