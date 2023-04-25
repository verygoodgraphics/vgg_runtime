#include "MainComposer.hpp"

#include "Sdk/VggSdk.hpp"
#include "VggSdkAddon.hpp"
#include "NativeExec.hpp"

void MainComposer::setup(const std::string& sdkUrl)
{
  auto env = std::make_shared<VggEnv>();
  auto js_engine = std::make_shared<NativeExec>();
  auto exec = std::make_shared<VggExec>(js_engine, env);

  js_engine->inject([](node::Environment* env) { link_vgg_sdk_addon(env); });
  setupVgg(exec, sdkUrl);

  VGG::DIContainer<std::shared_ptr<VggExec>>::get() = exec;
  VGG::DIContainer<std::shared_ptr<VggSdk>>::get().reset(new VggSdk);
}

void MainComposer::teardown()
{
  VGG::DIContainer<std::shared_ptr<VggExec>>::get().reset();
  VGG::DIContainer<std::shared_ptr<VggSdk>>::get().reset();
}

void MainComposer::setupVgg(std::shared_ptr<VggExec> vggExec, const std::string& sdkUrl)
{
  std::string set_vgg_code(R"(
    const { getVgg, getVggSdk, setVgg } = await import(")");
  set_vgg_code.append(sdkUrl);
  set_vgg_code.append(R"(");
    var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');
    setVgg(vggSdkAddon);
  )");

  vggExec->evalModule(set_vgg_code);
}