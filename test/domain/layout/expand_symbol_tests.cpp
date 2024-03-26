#include "Domain/Layout/ExpandSymbol.hpp"

#include "Domain/JsonSchemaValidator.hpp"
#include "Domain/Layout/Helper.hpp"
#include "Domain/Layout/Rule.hpp"
#include "Domain/Model/JsonKeys.hpp"
#include "Utility/VggFloat.hpp"

#include "domain/model/daruma_helper.hpp"
#include "test_config.hpp"

#include <gtest/gtest.h>

using namespace VGG;
using namespace VGG::Model;

constexpr auto EPSILON = 1e-6;

namespace VGG::Layout
{

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

  const nlohmann::json* layoutRule(const nlohmann::json& layoutJson, const std::string& id)
  {
    if (!layoutJson.is_object())
    {
      return nullptr;
    }

    auto& obj = layoutJson[K_OBJ];
    if (!obj.is_array())
    {
      return nullptr;
    }

    for (auto& item : obj)
    {
      auto& jsonId = item[K_ID];
      if (jsonId == id)
      {
        return &item;
      }
    }

    return nullptr;
  }

  bool hasLayoutRule(const nlohmann::json& layoutJson, const std::string& id)
  {
    return layoutRule(layoutJson, id) != nullptr;
  }
};

TEST_F(VggExpandSymbolTestSuite, Smoke)
{
  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  EXPECT_TRUE(result_json.is_object());
}

TEST_F(VggExpandSymbolTestSuite, fill_childObjects)
{
  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/childObjects" };
  auto                         instance_child = result_json[path];
  EXPECT_TRUE(instance_child.is_array());
}

TEST_F(VggExpandSymbolTestSuite, scale)
{ // Given
  std::string  filePath = "testDataDir/symbol/scale/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/1/childObjects/0/childObjects/1" };
  auto                         child = result_json[path];
  auto                         child_bounds = child[K_BOUNDS].get<Rect>();
  auto                         child_matrix = child[K_MATRIX].get<Matrix>();

  Rect   expect_bounds{ 0, 0, 163, 100 };
  Matrix expect_matrix{ 1, 0, 0, 1, 378.15999999999997, -208.66666666666663 };

  EXPECT_EQ(child_bounds, expect_bounds);
  EXPECT_EQ(child_matrix, expect_matrix);

  // Then
  nlohmann::json::json_pointer path2{
    "/frames/1/childObjects/0/childObjects/2/childObjects/0/shape/subshapes/0/subGeometry/points/2"
  };
  auto  point = result_json[path2][K_POINT].get<Point>();
  Point expect_point{ 704.15999999999997, -129.33333333333331 };
  EXPECT_EQ(point, expect_point);
}

TEST_F(VggExpandSymbolTestSuite, expand_masterId_overridden_instance)
{
  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/4/childObjects/1" };
  auto                         instance_child = result_json[path];
  EXPECT_TRUE(instance_child[K_MASTER_ID].is_null());
  EXPECT_TRUE(instance_child[K_OVERRIDE_VALUES].is_null());
}

TEST_F(VggExpandSymbolTestSuite, color_override)
{ // Given
  std::string  filePath = "testDataDir/symbol/scale/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{
    "/frames/1/childObjects/0/childObjects/2/childObjects/0/style/fills/0/color/blue"
  };
  double blue = result_json[path];

  EXPECT_NEAR(blue, 0.7517530913433672, EPSILON);
}

TEST_F(VggExpandSymbolTestSuite, override_with_star_wildcard)
{
  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  sut.run();

  // Then
  const auto& result = sut.designModel();

  // "/frames/0/childObjects/2/childObjects/0/fontAttr/0/fills/0/color/blue"
  auto& master = std::get<SymbolMaster>(result.frames[0].childObjects[2]);
  auto& text = std::get<Text>(master.childObjects[0]);
  auto  blue = text.fontAttr[0].fills.value()[0].color.value().blue;

  EXPECT_NEAR(blue, 0.01908801696712621, EPSILON);
}

TEST_F(VggExpandSymbolTestSuite, override_master_own_style)
{
  // Given
  std::string  filePath = "testDataDir/symbol/instance_override_master‘s_own_style/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/style/fills" };
  auto                         fills = result_json[path].dump();

  EXPECT_EQ(fills, "[]");
}

TEST_F(VggExpandSymbolTestSuite, override_multi_times)
{
  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
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
  std::string  filePath = "testDataDir/symbol/reference_override/design.json";
  auto         designJson = Helper::load_json(filePath);
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
  std::string  filePath = "testDataDir/symbol/invalid_override/design.json";
  auto         designJson = Helper::load_json(filePath);
  ExpandSymbol sut{ designJson };

  // When
  auto resultJson = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/1/childObjects/1/childObjects/0/style/fills" };
  auto&                        fills = resultJson[path];

  EXPECT_TRUE(fills.is_array());
  EXPECT_EQ(fills.size(), 1);
}

TEST_F(VggExpandSymbolTestSuite, BoundsOverride)
{
  // Given
  std::string  filePath = "testDataDir/symbol/BoundsOverrides/design.json";
  auto         designJson = Helper::load_json(filePath);
  ExpandSymbol sut{ designJson };

  // When
  auto resultJson = sut();

  // Then
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/6/childObjects/0/childObjects/0/childObjects/0/bounds"
    };
    auto&  bounds = resultJson[path];
    double width = bounds[K_WIDTH];
    double height = bounds[K_HEIGHT];
    EXPECT_DOUBLE_EQ(width, 102.578125);
    EXPECT_DOUBLE_EQ(height, 123.44138977905212);
  }
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/3/childObjects/0/childObjects/0/bounds"
    };
    auto&  bounds = resultJson[path];
    double width = bounds[K_WIDTH];
    double height = bounds[K_HEIGHT];
    EXPECT_DOUBLE_EQ(width, 193.64583333333331);
    EXPECT_DOUBLE_EQ(height, 83.012987012987011);
  }
  {
    nlohmann::json::json_pointer matrixPath{
      "/frames/0/childObjects/6/childObjects/0/childObjects/0/childObjects/1/matrix"
    };
    Matrix instanceMatrix = resultJson[matrixPath];
    Matrix matrix{ 1, 0, 0, 1, 149.92187215161994, 0 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
}

TEST_F(VggExpandSymbolTestSuite, unique_object_id_in_instance_tree)
{
  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  nlohmann::json::json_pointer path{ "/frames/0/childObjects/4/childObjects/1/childObjects/0/id" };
  auto                         id = result_json[path].get<std::string>();

  EXPECT_EQ(
    id,
    "98C9A450-A48C-4072-B310-E9E80A20F309__651675C7-452D-48D3-A84A-A6CF6796E3B3__95C02DB7-"
    "5EC9-4A61-97B1-0FFBE0C30E83");
}

TEST_F(VggExpandSymbolTestSuite, unique_id_in_mask_by)
{
  // Given
  std::string  filePath = "testDataDir/symbol/mask/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  {
    nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/childObjects/0/alphaMaskBy/0/id" };
    auto                         id = result_json[path].get<std::string>();
    EXPECT_EQ(id, "010894C5-A614-4918-A147-8ECF371856F3__82B564BA-4F87-47FE-9986-26B57D43B3EB");
  }
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/0/childObjects/1/childObjects/0/childObjects/0/outlineMaskBy/0"
    };
    auto id = result_json[path].get<std::string>();
    EXPECT_EQ(
      id,
      "010894C5-A614-4918-A147-8ECF371856F3__CE1AF72C-CEA2-4E11-B1F3-53881A6CA6B5__"
      "D0A3D25C-21D1-465C-AD31-5A1B56F69FB7");
  }
}

TEST_F(VggExpandSymbolTestSuite, validate_expanded_design_json)
{
  SKIP_SLOW_TEST;

  // Given
  std::string  filePath = "testDataDir/symbol/symbol_instance/design.json";
  auto         design_json = Helper::load_json(filePath);
  ExpandSymbol sut{ design_json };

  // When
  auto result_json = sut();

  // Then
  JsonSchemaValidator validator;
  json                schema = Helper::load_json("../../asset/vgg-format.json");
  validator.setRootSchema(schema);
  EXPECT_TRUE(validator.validate(result_json));
}

TEST_F(VggExpandSymbolTestSuite, ExpandLayout)
{
  // Given
  std::string  designFilePath = "testDataDir/instance_layout/1_instance_layout/design.json";
  std::string  layoutFilePath = "testDataDir/instance_layout/1_instance_layout/layout.json";
  auto         designJson = Helper::load_json(designFilePath);
  auto         layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  auto expandedLayoutJson = std::get<1>(result);

  // rule
  EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2"));
  EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2__1:3"));
  EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2__1:4"));

  // layout
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 900, 400 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 0, -410 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/childObjects/0/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ Point{ 0, 0 }, { 200, 150 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/childObjects/0/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 100, -40 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 200, 250 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 600, -40 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
}

TEST_F(VggExpandSymbolTestSuite, OverrideLayout)
{
  // Given
  std::string  designFilePath = "testDataDir/instance_layout/2_layout_overrides/design.json";
  std::string  layoutFilePath = "testDataDir/instance_layout/2_layout_overrides/layout.json";
  auto         designJson = Helper::load_json(designFilePath);
  auto         layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  auto expandedLayoutJson = std::get<1>(result);

  // layout
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 900, 400 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 0, -410 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/childObjects/0/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ Point{ 0, 0 }, { 200, 150 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/childObjects/0/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 240, -210 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 200, 250 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 460, -110 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
}

TEST_F(VggExpandSymbolTestSuite, NestedLayout)
{
  // Given
  std::string  designFilePath = "testDataDir/instance_layout/10_nested_instance/design.json";
  std::string  layoutFilePath = "testDataDir/instance_layout/10_nested_instance/layout.json";
  auto         designJson = Helper::load_json(designFilePath);
  auto         layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  auto expandedLayoutJson = std::get<1>(result);

  // rule
  {
    EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2__601:8"));
  }

  // layout
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 1000, 400 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 0, -410 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 400, 300 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 410, -40 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{
      "/frames/0/childObjects/1/childObjects/1/childObjects/0/bounds"
    };
    VGG::Layout::Rect instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect rect{ Point{ 0, 0 }, { 57, 110 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{
      "/frames/0/childObjects/1/childObjects/1/childObjects/0/matrix"
    };
    Matrix instanceMatrix = expandedDesignJson[matrixPath];
    Matrix matrix{ 1, 0, 0, 1, 40, -30 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
}

TEST_F(VggExpandSymbolTestSuite, NestedLayoutOverride)
{
  // Given
  std::string  designFilePath = "testDataDir/instance_layout/11_nested_override/design.json";
  std::string  layoutFilePath = "testDataDir/instance_layout/11_nested_override/layout.json";
  auto         designJson = Helper::load_json(designFilePath);
  auto         layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  auto expandedLayoutJson = std::get<1>(result);

  // rule
  {
    EXPECT_TRUE(hasLayoutRule(expandedLayoutJson, "401:2__601:8"));
  }

  // layout
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 1200, 400 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 0, -410 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{ "/frames/0/childObjects/1/childObjects/1/bounds" };
    VGG::Layout::Rect            instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect            rect{ { 0, 0 }, { 400, 300 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{ "/frames/0/childObjects/1/childObjects/1/matrix" };
    Matrix                       instanceMatrix = expandedDesignJson[matrixPath];
    Matrix                       matrix{ 1, 0, 0, 1, 700, -40 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
  {
    nlohmann::json::json_pointer boundsPath{
      "/frames/0/childObjects/1/childObjects/1/childObjects/0/bounds"
    };
    VGG::Layout::Rect instanceBounds = expandedDesignJson[boundsPath];
    VGG::Layout::Rect rect{ Point{ 0, 0 }, { 57, 110 } };
    EXPECT_EQ(instanceBounds, rect);

    nlohmann::json::json_pointer matrixPath{
      "/frames/0/childObjects/1/childObjects/1/childObjects/0/matrix"
    };
    Matrix instanceMatrix = expandedDesignJson[matrixPath];
    Matrix matrix{ 1, 0, 0, 1, 303, -112 };
    EXPECT_EQ(instanceMatrix, matrix);
  }
}

TEST_F(VggExpandSymbolTestSuite, OwnOverrideKey)
{
  // Given
  std::string  designFilePath = "testDataDir/symbol/own_overrideKey/design.json";
  std::string  layoutFilePath = "testDataDir/symbol/own_overrideKey/layout.json";
  auto         designJson = Helper::load_json(designFilePath);
  auto         layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  auto expandedLayoutJson = std::get<1>(result);

  // layout
  {
    nlohmann::json::json_pointer visiblePath{
      "/frames/0/childObjects/0/childObjects/0/childObjects/1/visible"
    };
    bool visible = expandedDesignJson[visiblePath];
    EXPECT_EQ(visible, false);
  }
  {
    nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/childObjects/0/bounds" };
    auto                         childBounds = expandedDesignJson[path].get<Rect>();
    Rect                         expectBounds{ 0, 0, 237, 38 };
    EXPECT_EQ(childBounds, expectBounds);
  }
}

TEST_F(VggExpandSymbolTestSuite, layout_container_when_child_bounds_are_overridden)
{
  // Given
  std::string designFilePath =
    "testDataDir/symbol/layout_container_when_child_bounds_are_overridden/design.json";
  std::string layoutFilePath =
    "testDataDir/symbol/layout_container_when_child_bounds_are_overridden/layout.json";
  auto         designJson = Helper::load_json(designFilePath);
  auto         layoutJson = Helper::load_json(layoutFilePath);
  ExpandSymbol sut{ designJson, layoutJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  auto expandedLayoutJson = std::get<1>(result);

  // layout
  {
    nlohmann::json::json_pointer nodePath{ "/frames/0/childObjects/0/childObjects/0" };
    { // bounds
      auto path = nodePath / "bounds";
      auto bounds = expandedDesignJson[path].get<Rect>();
      EXPECT_DOUBLE_EQ(bounds.width(), 61.0);
      EXPECT_DOUBLE_EQ(bounds.height(), 20.0);
    }
    { // matrix
      auto path = nodePath / "matrix";
      auto childMatrix = expandedDesignJson[path].get<Matrix>();
      EXPECT_DOUBLE_EQ(childMatrix.tx, 8.0);
      EXPECT_DOUBLE_EQ(childMatrix.ty, -2.0);
    }
  }
  {
    nlohmann::json::json_pointer nodePath{
      "/frames/0/childObjects/0/childObjects/1/childObjects/0"
    };
    { // bounds
      auto path = nodePath / "bounds";
      auto bounds = expandedDesignJson[path].get<Rect>();
      EXPECT_DOUBLE_EQ(bounds.height(), 7.2767872066322727);
      EXPECT_DOUBLE_EQ(bounds.width(), 7.0671430843512839);
    }
    { // matrix
      auto path = nodePath / "matrix";
      auto childMatrix = expandedDesignJson[path].get<Matrix>();
      EXPECT_DOUBLE_EQ(childMatrix.tx, 1.461796760559082);
      EXPECT_DOUBLE_EQ(childMatrix.ty, -1.3610985279083252);
    }
  }
  { // layout
    auto json = layoutRule(expandedLayoutJson, "1:38__1:25");
    EXPECT_NE(json, nullptr);
    VGG::Layout::Internal::Rule::Rule rule = *json;
    EXPECT_EQ(rule.width.value.types, Internal::Rule::Length::ETypes::PX);
    EXPECT_DOUBLE_EQ(rule.width.value.value, 61.0);
  }
}

TEST_F(VggExpandSymbolTestSuite, VectorNetworkGroupOverride)
{
  // Given
  std::string  designFilePath = "testDataDir/symbol/vector_network/design.json";
  auto         designJson = Helper::load_json(designFilePath);
  ExpandSymbol sut{ designJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/0/childObjects/0/childObjects/0/style/borders/0/thickness"
    };
    double value = expandedDesignJson[path];
    EXPECT_DOUBLE_EQ(value, 10);
  }
  {
    nlohmann::json::json_pointer path{ "/frames/0/childObjects/0/childObjects/0/childObjects/1/"
                                       "shape/subshapes/0/subGeometry/style/borders/0/thickness" };
    double                       value = expandedDesignJson[path];
    EXPECT_DOUBLE_EQ(value, 10);
  }
}

TEST_F(VggExpandSymbolTestSuite, BoundsOverrideOrderRootFirst)
{
  // Given
  std::string  designFilePath = "testDataDir/symbol/bounds_overrides_order/design.json";
  auto         designJson = Helper::load_json(designFilePath);
  ExpandSymbol sut{ designJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/0/childObjects/0/childObjects/1/bounds"
    };
    auto bounds = expandedDesignJson[path].get<Rect>();
    EXPECT_DOUBLE_EQ(bounds.height(), 278.19389205276883);
    EXPECT_DOUBLE_EQ(bounds.width(), 120.29999542236328);
  }
}

TEST_F(VggExpandSymbolTestSuite, VariableBool)
{
  // Given
  std::string  designFilePath = "testDataDir/symbol/component_property/bool/design.json";
  auto         designJson = Helper::load_json(designFilePath);
  ExpandSymbol sut{ designJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/1/childObjects/0/childObjects/0/visible"
    };
    bool visible = expandedDesignJson[path];
    EXPECT_FALSE(visible);
  }
  {
    nlohmann::json::json_pointer path{ "/frames/0/childObjects/2/childObjects/0/visible" };
    bool                         visible = expandedDesignJson[path];
    EXPECT_TRUE(visible);
  }
}

TEST_F(VggExpandSymbolTestSuite, VariableText)
{
  // Given
  std::string  designFilePath = "testDataDir/symbol/component_property/text/design.json";
  auto         designJson = Helper::load_json(designFilePath);
  ExpandSymbol sut{ designJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  {
    nlohmann::json::json_pointer path{ "/frames/0/childObjects/1/childObjects/0/content" };
    std::string                  text = expandedDesignJson[path];
    EXPECT_TRUE(text == "var text in instance");
  }
}

TEST_F(VggExpandSymbolTestSuite, VariableString)
{
  // Given
  std::string  designFilePath = "testDataDir/symbol/component_property/string/design.json";
  auto         designJson = Helper::load_json(designFilePath);
  ExpandSymbol sut{ designJson };

  // When
  auto result = sut.run();

  // Then
  auto expandedDesignJson = std::get<0>(result);
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/3/childObjects/0/childObjects/0/name"
    };
    std::string name = expandedDesignJson[path];
    EXPECT_TRUE(name == "Star 1");
  }
  {
    nlohmann::json::json_pointer path{
      "/frames/0/childObjects/5/childObjects/0/childObjects/0/childObjects/0/name"
    };
    std::string name = expandedDesignJson[path];
    EXPECT_TRUE(name == "Star 1");
  }
}

} // namespace VGG::Layout