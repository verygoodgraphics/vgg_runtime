#include "Adapter/NativeExec.hpp"

#include "Log.h"
#include "test_config.hpp"

#include <gtest/gtest.h>

#include <memory>

class VggNativeExecTestSuite : public ::testing::Test
{
protected:
  NativeExec sut;
  NativeExec* sut_ptr = &sut;

  void SetUp() override
  {
  }

  void TearDown() override
  {
  }
};

TEST_F(VggNativeExecTestSuite, Do_nothing)
{
  GTEST_SUCCEED();
}

TEST_F(VggNativeExecTestSuite, Smoke)
{
  // Given

  // When
  auto result = sut_ptr->evalScript("1");

  // Then
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Not_abort)
{
  // When
  auto code = R"(
    var process = require('process');
    if(false){
      process.abort();
    }
  )";
  auto result = sut_ptr->evalScript(code);

  // Then
  GTEST_SUCCEED();
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
  GTEST_SKIP() << "Skipping sometimes block test";

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
  GTEST_SKIP() << "Skipping run js error test";

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
  GTEST_SKIP() << "Skipping run js error test";

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

TEST_F(VggNativeExecTestSuite, Internal_evalScript_import_data)
{
  // tmp code
  auto code = R"(
    console.log('eval script 1');
    const { evalScript } = require('internal/process/execution');
    const code = "import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);";
    evalScript('fakeName', code);
  )";
  auto result = sut_ptr->evalScript(code);
  EXPECT_EQ(result, true);

  // // use evalScript setupped in NativeExec
  // auto code2 = R"(
  //   console.log('eval script 2');
  //   const code2 =
  //   "import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);";
  //   evalScript('fakeName', code2);
  // )";
  // result = sut_ptr->evalScript(code2);
  // EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Internal_evalModule_hello)
{
  auto code = R"(
    const { evalModule } = require('internal/process/execution');
    const code = 'console.log("hello, evalModule")';
    evalModule(code);
  )";
  auto result = sut_ptr->evalScript(code);
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Internal_evalModule_enable_network_import_local_http_url)
{
  GTEST_SKIP() << "Skipping local http url test";

  auto code = R"(
    const { evalModule } = require('internal/process/execution');
    const code = 'import("http://localhost:8000/mjs/vgg-di-container.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    evalModule(code);
  )";
  auto result = sut_ptr->evalScript(code);
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Internal_evalModule_1)
{
  SKIP_S3_DEPENDENT_TEST

  auto code = R"(
    const { evalModule } = require('internal/process/execution');
    const code2 = 'import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    evalModule(code2);
  )";
  auto result = sut_ptr->evalScript(code);
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Internal_evalModule_2)
{
  SKIP_S3_DEPENDENT_TEST

  auto code = R"(
    const code3 = `
      const { evalModule } = require('internal/process/execution');
      const code2 = 'import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
      evalModule(code2);
      `;
    require('vm').runInThisContext(code3);
  )";

  auto result = sut_ptr->evalScript(code);
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Eval_module_smoke)
{
  auto code = "console.log('Hello everyone!');";
  auto result = sut_ptr->evalModule(code);
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, Eval_module_await_import)
{
  SKIP_S3_DEPENDENT_TEST

  auto code = R"(
const { getVgg, getVggSdk, setVgg } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
console.log('#vgg is: ', getVgg());
  )";
  auto result = sut_ptr->evalModule(code);
  EXPECT_EQ(result, true);
}

TEST_F(VggNativeExecTestSuite, setInterval)
{
  auto code = "let times = 0; setInterval(() => { console.log('test, ', ++times);}, 1000)";
  auto result = sut_ptr->evalModule(code);
  EXPECT_EQ(result, true);
}
