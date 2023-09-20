#include "Domain/Layout/ExpandSymbol.hpp"

#include "Domain/JsonSchemaValidator.hpp"
#include "Domain/Layout/Helper.hpp"
#include "Domain/Model/JsonKeys.hpp"
#include "Utility/VggFloat.hpp"

#include "domain/model/daruma_helper.hpp"
#include "test_config.hpp"

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

  bool hasLayoutRule(const nlohmann::json& layoutJson, const std::string& id)
  {
    if (!layoutJson.is_object())
    {
      return false;
    }

    auto& obj = layoutJson["obj"];
    if (!obj.is_array())
    {
      return false;
    }

    for (auto& item : obj)
    {
      auto& jsonId = item[K_ID];
      if (jsonId == id)
      {
        return true;
      }
    }

    return false;
  }
};

TEST_F(VggExpandSymbolTestSuite, Smoke)
{
  // Given
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  EXPECT_TRUE(result_json.is_object());
}

TEST_F(VggExpandSymbolTestSuite, fill_childObjects)
{
  // Given
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
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
  std::string filePath = "testDataDir/symbol/scale/design.json";
  auto design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/1/childObjects/0/childObjects/1" };
  auto child = result_json[path];
  auto child_bounds = child[K_BOUNDS].get<Rect>();
  auto child_frame = child[K_FRAME].get<Rect>();
  auto child_matrix = child[K_MATRIX].get<Matrix>();

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
  auto point = result_json[path2][K_POINT].get<Point>();
  Point expect_point{ 704.1600341796875, -129.33334350585938 };
  EXPECT_EQ(point, expect_point);
}

TEST_F(VggExpandSymbolTestSuite, expand_masterId_overridden_instance)
{
  // Given
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/4/childObjects/1" };
  auto instance_child = result_json[path];
  EXPECT_TRUE(instance_child[K_MASTER_ID].is_null());
  EXPECT_TRUE(instance_child[K_OVERRIDE_VALUES].is_null());
}

TEST_F(VggExpandSymbolTestSuite, color_override)
{ // Given
  std::string filePath = "testDataDir/symbol/scale/design.json";
  auto design_json = Helper::load_json(filePath);
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
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
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
  std::string filePath = "testDataDir/symbol/instance_override_master‘s_own_style/design.json";
  auto design_json = Helper::load_json(filePath);
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
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
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

TEST_F(VggExpandSymbolTestSuite, ReferenceStyleOverride)
{
  // Given
  std::string filePath = "testDataDir/symbol/reference_override/design.json";
  auto designJson = Helper::load_json(filePath);
  ExpandSymbol sut{ designJson };

  // When
  auto resultJson = sut();

  // Then
  {
    nlohmann::json::json_pointer path{
      "/frames/1/childObjects/0/childObjects/2/childObjects/0/style/blurs/0/center/0"
    };
    double centerX = resultJson[path];

    EXPECT_DOUBLE_EQ(centerX, 0.5);
  }
  {
    nlohmann::json::json_pointer path{
      "/frames/1/childObjects/0/childObjects/2/childObjects/0/contextSettings/opacity"
    };
    double opacity = resultJson[path];

    EXPECT_DOUBLE_EQ(opacity, 1.0);
  }
}

TEST_F(VggExpandSymbolTestSuite, IgnoreInvalideArrayIndexOverride)
{
  // Given
  std::string filePath = "testDataDir/symbol/invalid_override/design.json";
  auto designJson = Helper::load_json(filePath);
  ExpandSymbol sut{ designJson };

  // When
  auto resultJson = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/1/childObjects/1/childObjects/0/style/fills" };
  auto& fills = resultJson[path];

  EXPECT_TRUE(fills.is_array());
  EXPECT_EQ(fills.size(), 1);
}

TEST_F(VggExpandSymbolTestSuite, BoundsOverride)
{
  // Given
  std::string filePath = "testDataDir/symbol/BoundsOverrides/design.json";
  auto designJson = Helper::load_json(filePath);
  ExpandSymbol sut{ designJson };

  // When
  auto resultJson = sut();

  // Then
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/6/childObjects/0/childObjects/0/childObjects/0/bounds"
    };
    auto& bounds = resultJson[path];
    double width = bounds[K_WIDTH];
    double height = bounds[K_HEIGHT];
    EXPECT_DOUBLE_EQ(width, 102.578125);
    EXPECT_DOUBLE_EQ(height, 123.4413833618164);
  }
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/3/childObjects/0/childObjects/0/bounds"
    };
    auto& bounds = resultJson[path];
    double width = bounds[K_WIDTH];
    double height = bounds[K_HEIGHT];
    EXPECT_DOUBLE_EQ(width, 193.6458282470703);
    EXPECT_DOUBLE_EQ(height, 83.01298522949219);
  }
}

TEST_F(VggExpandSymbolTestSuite, unique_object_id_in_instance_tree)
{
  // Given
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
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

TEST_F(VggExpandSymbolTestSuite, unique_id_in_mask_by)
{
  // Given
  std::string filePath = "testDataDir/symbol/mask/design.json";
  auto design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  {
    nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/childObjects/0/alphaMaskBy/0/id" };
    auto id = result_json[path].get<std::string>();
    EXPECT_EQ(id, "010894C5-A614-4918-A147-8ECF371856F3__82B564BA-4F87-47FE-9986-26B57D43B3EB");
  }
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/0/childObjects/1/childObjects/0/childObjects/0/outlineMaskBy/0"
    };
    auto id = result_json[path].get<std::string>();
    EXPECT_EQ(id,
              "010894C5-A614-4918-A147-8ECF371856F3__CE1AF72C-CEA2-4E11-B1F3-53881A6CA6B5__"
              "D0A3D25C-21D1-465C-AD31-5A1B56F69FB7");
  }
}

TEST_F(VggExpandSymbolTestSuite, validate_expanded_design_json)
{
  SKIP_SLOW_TEST;

  // Given
  std::string filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  JsonSchemaValidator validator;
  json schema = Helper::load_json("../../asset/vgg-format.json");
  validator.setRootSchema(schema);
  EXPECT_TRUE(validator.validate(result_json));
}

TEST_F(VggExpandSymbolTestSuite, ExpandLayout)
{
  // Given
  std::string designFilePath = "testDataDir/instance_layout/1_instance_layout/design.json";
  std::string layoutFilePath = "testDataDir/instance_layout/1_instance_layout/layout.json";
  auto designJson = Helper::load_json(designFilePath);
  auto layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  Helper::write_json(expandedDesignJson,
                     "/Users/houguanhua/code/vgg/vgg_runtime_B/test/testDataDir/instance_layout/"
                     "1_instance_layout/_design.json");
  auto expandedLayoutJson = std::get<1>(result);
  Helper::write_json(expandedDesignJson,
                     "/Users/houguanhua/code/vgg/vgg_runtime_B/test/testDataDir/instance_layout/"
                     "1_instance_layout/_layout.json");

  EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2"));
  EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2__1:3"));
  EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2__1:4"));
}