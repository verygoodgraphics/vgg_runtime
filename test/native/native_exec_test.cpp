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
  auto result = sut_ptr->evalScript("1");

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Eval_script_console_log)
{
  // When
  auto result = sut_ptr->evalScript("console.log('hello, world')");

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, VM_script_run_console_log)
{
  // Given
  auto code = R"(
    const vm = require('vm');
    const code = "console.log('log in vm run');";
    const s1 = new vm.Script(code);
    s1.runInThisContext();
  )";

  // When
  auto result = sut_ptr->evalScript(code);

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, VM_run_script_console_log)
{
  // Given
  auto code = R"(
    const vm = require('vm');
    const code = "console.log('log in vm run');";
    var context = vm.createContext(globalThis);
    vm.runInContext(code, context);
  )";

  // When
  auto result = sut_ptr->evalScript(code);

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Eval_dynamic_import_data)
{
  // Given
  auto code = R"(
import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);
  )";

  // When
  auto result = sut_ptr->evalScript(code);

  // Then
  EXPECT_EQ(result, true);

  // TypeError: Invalid host defined options
}

TEST_F(VggNativeExecTestSuite, VM_script_run_dynamic_import_data)
{
  // Given
  auto code = R"(
    const vm = require('vm');
    const code = "import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);";
    const s1 = new vm.Script(code);

    // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not specified.
    // s1.runInThisContext();

    var context = vm.createContext(globalThis);
    s1.runInContext(context);
  )";

  // When
  auto result = sut_ptr->evalScript(code);

  // Then
  EXPECT_EQ(result, true);
  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST_F(VggNativeExecTestSuite, VM_run_dynamic_import_data)
{
  // Given
  auto code = R"(
    const vm = require('vm');
    const code = "import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);";
    var context = vm.createContext(globalThis);
    vm.runInContext(code, context);
  )";

  // When
  auto result = sut_ptr->evalScript(code);

  // Then
  EXPECT_EQ(result, true);

  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST_F(VggNativeExecTestSuite, Eval_dynamic_import_file)
{
  // Given
  auto code = R"(
import("./testDataDir/js/test-esm-ok.mjs");
  )";

  // When
  auto result = sut_ptr->evalScript(code);

  // Then
  EXPECT_EQ(result, true);

  // TypeError: Invalid host defined options
  // https://github.com/nodejs/node/issues?q=is%3Aissue+Invalid+host+defined+options+is%3Aopen
}