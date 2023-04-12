#include "NativeExec.hpp"

#include "VggDepContainer.hpp"
#include "VggSdk.hpp"
#include "VggSdkAddon.hpp"
#include "VggSdkMock.hpp"

#include <gtest/gtest.h>

#include <memory>

class VggNativeSdkTestSuite : public ::testing::Test
{
protected:
  std::shared_ptr<NativeExec> m_sut_ptr;
  VggSdkMock* m_mock_sdk_ptr = nullptr;

  void SetUp() override
  {
    m_sut_ptr.reset(new NativeExec);
    m_mock_sdk_ptr = new VggSdkMock();
    VggDepContainer<std::shared_ptr<VggSdk>>::get().reset(m_mock_sdk_ptr);

    m_sut_ptr->inject([](node::Environment* env) { link_vgg_sdk_addon(env); });
    injectVgg();
  }

  void TearDown() override
  {
    m_sut_ptr.reset();
    m_mock_sdk_ptr = nullptr;
    VggDepContainer<std::shared_ptr<VggSdk>>::get().reset();
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
