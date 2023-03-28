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
