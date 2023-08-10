#include "Domain/Layout/ExpandSymbol.hpp"

#include "domain/model/daruma_helper.hpp"

#include <gtest/gtest.h>

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

// no overide
// overide width & height

// instance of one master containing instance have no overide
// instance of one master containing instance overide width & height