#include <gtest/gtest.h>

#if 0
// https://google.github.io/googletest/advanced.html#skipping-test-execution
#define SKIP_FOR_DEBUG GTEST_SKIP() << "Skipping for debug";
#else
#define SKIP_FOR_DEBUG
#endif

extern int call_main(int argc, char** argv);

TEST(NodeTest, Console)
{
  SKIP_FOR_DEBUG

  const char* argv[] = { "", "console.log('hello world')" };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
}

TEST(NodeTest, Require_excution)
{
  auto code = R"(
    const { evalScript } = require('internal/process/execution');
    console.log("#requrie, evalScript is: ", evalScript);
  )";
  const char* argv[] = { "", "--expose-internals", code };
  auto ret = call_main(3, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
}

TEST(NodeTest, Internal_evalScript_import_data)
{
  auto code = R"(
    const { evalScript } = require('internal/process/execution');
    const code = "import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);";
    evalScript('fakeName', code);
  )";
  const char* argv[] = { "", "--expose-internals", code };
  auto ret = call_main(3, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
}

TEST(NodeTest, Internal_evalScript_import_file)
{
  auto code = R"(
    const { evalScript } = require('internal/process/execution');
    const code = 'import("./testDataDir/js/test-esm-ok.mjs").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    evalScript('fakeName', code);
  )";
  const char* argv[] = { "", "--expose-internals", code };
  auto ret = call_main(3, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
}

TEST(NodeTest, Internal_evalModule1)
{
  GTEST_SKIP() << "Skipping exit test";

  auto code = R"(
    const { evalModule } = require('internal/process/execution');
    const code = 'import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    evalModule(code);
  )";
  const char* argv[] = { "", "--expose-internals", code };
  auto ret = call_main(3, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
  // Error [ERR_UNSUPPORTED_ESM_URL_SCHEME]: Only URLs with a scheme in: file and data are supported
  // by the default ESM loader. Received protocol 'http:'
}

TEST(NodeTest, Internal_evalModule_enable_network_import)
{
  GTEST_SKIP() << "Skipping exit test";

  auto code = R"(
    const { evalModule } = require('internal/process/execution');
    const code = 'import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    evalModule(code);
  )";
  const char* argv[] = { "", "--expose-internals", "--experimental-network-imports", code };
  auto ret = call_main(4, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
  // Error [ERR_NETWORK_IMPORT_DISALLOWED]: import of 'http://s3.vgg.cool/test/js/vgg-sdk.esm.js' by
  // undefined is not supported: http can only be used to load local resources (use https instead).
}

TEST(NodeTest, Internal_evalModule_custom_http_loader)
{
  auto code = R"(
    const { evalModule } = require('internal/process/execution');
    const code = 'import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    evalModule(code);
  )";
  const char* argv[] = { "",
                         "--expose-internals",
                         "--experimental-loader",
                         "./testDataDir/js/http-loader.mjs",
                         code };
  auto ret = call_main(5, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
}

TEST(NodeTest, Internal_evalScript_custom_http_loader)
{
  GTEST_SKIP() << "Skipping exit test";

  auto code = R"(
    const code = 'import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    const { evalScript } = require('internal/process/execution');
    evalScript('fakeName', code);
  )";
  const char* argv[] = { "",
                         "--expose-internals",
                         "--experimental-loader",
                         "./testDataDir/js/http-loader.mjs",
                         code };
  auto ret = call_main(5, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
  // Error [ERR_UNSUPPORTED_ESM_URL_SCHEME]: Only URLs with a scheme in: file and data are supported
  // by the default ESM loader. Received protocol 'http:'
}

TEST(NodeTest, Internal_evalScript_custom_http_loader_enable_network_imports)
{
  GTEST_SKIP() << "Skipping exit test";

  // custom loader is useless for evalScript
  auto code = R"(
    const code = 'import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js").then((theModule)=>{ console.log("#theModule is: ", theModule); })';
    const { evalScript } = require('internal/process/execution');
    evalScript('fakeName', code);
  )";
  const char* argv[] = { "",
                         "--expose-internals",
                         "--experimental-network-imports",
                         "--experimental-loader",
                         "./testDataDir/js/http-loader.mjs",
                         code };
  auto ret = call_main(6, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
  // Error [ERR_NETWORK_IMPORT_DISALLOWED]: import of 'http://s3.vgg.cool/test/js/vgg-sdk.esm.js' by
  // undefined is not supported: http can only be used to load local resources (use https instead).
}

TEST(NodeTest, VM)
{
  GTEST_SKIP() << "Skipping exit test";

  auto code = R"(
    const vm = require('vm');
    // const code = "console.log('log in vm run');";
    const code = "import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);";
    var context = vm.createContext(globalThis);
    vm.runInContext(code, context);
  )";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST(NodeTest, Dynamic_import_data)
{
  GTEST_SKIP() << "Skipping exit test";

  // https://github.com/nodejs/node/issues/30591
  auto code = R"(
import('data:text/javascript,console.log(Date.now())').then(console.log).catch(console.error);
  )";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);

  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST(NodeTest, Dynamic_import_file0)
{
  GTEST_SKIP() << "Skipping exit test";

  // https://github.com/nodejs/node/issues/43280
  auto code = R"(
import("testDataDir/js/test-esm-ok.mjs");
  )";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);

  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST(NodeTest, Dynamic_import_file1)
{
  GTEST_SKIP() << "Skipping exit test";

  // https://github.com/nodejs/node/issues/43280
  auto code = R"(
import("js/test-esm-ok.mjs").then((theModule)=>{
  console.log('#theModule is: ', theModule);
});
  )";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);

  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST(NodeTest, Dynamic_import_file2)
{
  GTEST_SKIP() << "Skipping exit test";

  // https://github.com/nodejs/node/issues/43280
  auto code = R"(
const code = `
import("js/test-esm-ok.mjs").then((theModule)=>{
  console.log('#theModule is: ', theModule);
});
});
`;
import(`data:text/javascript,${encodeURIComponent(code)}`).then((m)=>{});
  )";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);

  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST(NodeTest, Import_file)
{
  GTEST_SKIP() << "Skipping exit test";

  auto code = R"(
function myStrictFunction() {
  "use strict";
import defaultExport from "js/test-esm-ok.mjs";
console.log('#defaultExport is: ', defaultExport);
}
myStrictFunction();
)";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);

  // SyntaxError: Cannot use import statement outside a module
}

TEST(NodeTest, ImportUrl)
{
  GTEST_SKIP() << "Skipping exit test";

  auto code = R"(
    // [SyntaxError: await is only valid in async functions and the top level bodies of modules]
// const vggModule = await import("http://s3.vgg.cool/test/js/vgg-sdk.esm.js");
// console.log('#vggModule is: ', vggModule);

const vggSdkUrl = "http://s3.vgg.cool/test/js/vgg-sdk.esm.js";
import(vggSdkUrl).then((container) => {
  container.getVgg().then((vgg) => {
    console.log('#vgg is: ', vgg);
  });
});

  )";
  const char* argv[] = { "", code };
  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);

  // TypeError [ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING]: A dynamic import callback was not
  // specified.
}

TEST(NodeTest, EvalFetchWasmAndCall)
{
  SKIP_FOR_DEBUG

  const char* argv[] = { "", R"(
const importObject = {
imports: {
    imported_func: arg => {
    console.log(arg);
    }
}
};

WebAssembly.instantiateStreaming(fetch("https://mdn.github.io/webassembly-examples/js-api-examples/simple.wasm"), importObject)
.then(obj => {
obj.instance.exports.exported_func();
});
  )" };

  auto ret = call_main(2, const_cast<char**>(argv));
  EXPECT_TRUE(0 == ret);
}
