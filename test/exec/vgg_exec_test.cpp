#include "VggExec.hpp"

#include "VggEnvMock.hpp"
#include "VggJSEngineMock.hpp"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

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
  auto mock_env = new VggEnvMock();
  auto mock_js_engine = new VggJSEngineMock();

  std::shared_ptr<VggEnv> env_ptr;
  std::shared_ptr<VggJSEngine> js_ptr;
  env_ptr.reset(mock_env);
  js_ptr.reset(mock_js_engine);

  VggExec sut(js_ptr, env_ptr);

  EXPECT_CALL(*mock_env, getEnv()).WillRepeatedly(Return("1"));
  EXPECT_CALL(*mock_js_engine, eval(_, _, _)).WillOnce(Return(true)).WillOnce(Return(false));

  // When
  auto result = sut.eval("1");
  // Then
  EXPECT_EQ(result, true);

  // When
  result = sut.eval("1");
  // Then
  EXPECT_EQ(result, false);
}