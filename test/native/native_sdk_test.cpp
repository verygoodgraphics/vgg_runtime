#include "Adapter/NativeExec.hpp"

#include "Adapter/VggSdkAddon.hpp"
#include "Application/VggSdk.hpp"
#include "VggSdkMock.hpp"
#include "test_config.hpp"

#include <gtest/gtest.h>

#include <memory>

using namespace VGG;

class VggNativeSdkTestSuite : public ::testing::Test
{
protected:
  NativeExec* m_sut_ptr;
  VggSdkMock* m_mock_sdk_ptr = nullptr;

  void SetUp() override
  {
    SKIP_S3_DEPENDENT_TEST

    m_sut_ptr = NativeExec::sharedInstance().get();
    m_mock_sdk_ptr = new VggSdkMock();

    m_sut_ptr->inject([](node::Environment* env) { link_vgg_sdk_addon(env); });
    injectVgg();
  }

  void TearDown() override
  {
    m_mock_sdk_ptr = nullptr;
  }

private:
  void injectVgg()
  {
    auto code = R"(
const { getVgg, getVggSdk, setVgg } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');
setVgg(vggSdkAddon);
  )";
    EXPECT_EQ(true, m_sut_ptr->evalModule(code));
  }
};

TEST_F(VggNativeSdkTestSuite, Smoke)
{
  auto code = R"(
const { getVgg, getVggSdk, setVgg } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
var vgg = await getVgg();
console.log('#vgg is: ', vgg);
  )";
  EXPECT_EQ(true, m_sut_ptr->evalModule(code));
}

TEST_F(VggNativeSdkTestSuite, Get_vgg_set_in_cpp)
{
  auto code = R"(
const { getVgg, getVggSdk, setVgg } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
var vgg = await getVgg();
if(!vgg){
  console.log('#vgg not ready, abort');
  require('process').abort();
}
console.log('#vgg is: ', vgg);
  )";
  EXPECT_EQ(true, m_sut_ptr->evalModule(code));
}

TEST_F(VggNativeSdkTestSuite, Get_sdk)
{
  auto code = R"(
    const { getVgg, getVggSdk, setVgg } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
    var vggSdk = await getVggSdk();

    if(!vggSdk){
    }
    console.log('#vggSdk is: ', vggSdk);
  )";
  EXPECT_EQ(true, m_sut_ptr->evalModule(code));
}

TEST_F(VggNativeSdkTestSuite, Sdk_smoke)
{
  VggSdkMock m;
  EXPECT_CALL(*m_mock_sdk_ptr, updateStyle()).Times(1);

  auto code = R"(
    const { getVggSdk } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
    var vggSdk = await getVggSdk();
    vggSdk.updateStyle();
  )";

  EXPECT_EQ(true, m_sut_ptr->evalModule(code));
}
