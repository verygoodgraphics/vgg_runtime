#include "MainComposer.hpp"

#include "VggSdkAddon.hpp"
#include "VggSdk.hpp"

void MainComposer::setup()
{
  auto env = std::make_shared<VggEnv>();
  auto js_engine = std::make_shared<NativeExec>();
  auto exec = std::make_shared<VggExec>(js_engine, env);

  js_engine->inject([](node::Environment* env) { link_vgg_sdk_addon(env); });
  setupVgg(exec);

  VggDepContainer<std::shared_ptr<VggExec>>::get() = exec;
  VggDepContainer<std::shared_ptr<VggSdk>>::get().reset(new VggSdk);
}

void MainComposer::teardown()
{
  VggDepContainer<std::shared_ptr<VggExec>>::get().reset();
  VggDepContainer<std::shared_ptr<VggSdk>>::get().reset();
}

void MainComposer::setupVgg(std::shared_ptr<VggExec> vggExec)
{
  auto code = R"(
    const { getVgg, getVggSdk, setVgg } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
    var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');
    setVgg(vggSdkAddon);
  )";
  vggExec->evalModule(code);
}