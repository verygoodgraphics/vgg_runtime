#include "NativeExec.hpp"

#include <gtest/gtest.h>

class VggNativeExecTestSuite : public ::testing::Test
{
protected:
  NativeExec sut;
  VggJSEngine* sut_ptr = &sut;

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

  // When
  auto result = sut_ptr->eval("1");

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Console)
{
  // When
  auto result = sut_ptr->eval("console.log('hello, world')");

  // Then
  EXPECT_EQ(result, true);
}
