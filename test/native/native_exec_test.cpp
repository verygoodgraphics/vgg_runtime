#include "NativeExec.hpp"

#include <gtest/gtest.h>

class VggNativeExecTestSuite : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }
  void TearDown() override
  {
  }
};

TEST_F(VggNativeExecTestSuite, Smoke)
{
  // Given
  NativeExec sut;
  VggJSEngine* sut_ptr = &sut;

  // When
  auto result = sut_ptr->eval("1");

  // Then
  EXPECT_EQ(result, true);
}