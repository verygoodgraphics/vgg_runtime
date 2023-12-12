#include "Domain/VggExec.hpp"

#include "Application/VggEnv.hpp"
#include "VggJSEngineMock.hpp"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

using namespace VGG;

class VggExecTestSuite : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }
  void TearDown() override
  {
  }
};

TEST_F(VggExecTestSuite, Smoke)
{
  // Given
  auto mock_env = new VggEnv();
  auto mock_js_engine = new VggJSEngineMock();

  std::shared_ptr<IVggEnv>     env_ptr;
  std::shared_ptr<VggJSEngine> js_ptr;
  env_ptr.reset(mock_env);
  js_ptr.reset(mock_js_engine);

  VggExec sut(js_ptr, env_ptr);

  EXPECT_CALL(*mock_js_engine, evalScript(_))
    .WillOnce(Return(true)) // setEnv
    .WillOnce(Return(true))
    .WillOnce(Return(true)) // setEnv
    .WillOnce(Return(false));

  // When
  auto result = sut.evalScript("1");
  // Then
  EXPECT_EQ(result, true);

  // When
  result = sut.evalScript("1");
  // Then
  EXPECT_EQ(result, false);
}