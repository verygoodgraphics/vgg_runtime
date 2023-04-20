#include "MainComposer.hpp"

#include "VggSdkAddon.hpp"
#include "VggSdk.hpp"

void MainComposer::setup(const std::string& sdkUrl)
{
  auto env = std::make_shared<VggEnv>();
  auto js_engine = std::make_shared<NativeExec>();
  auto exec = std::make_shared<VggExec>(js_engine, env);

  js_engine->inject([](node::Environment* env) { link_vgg_sdk_addon(env); });
  setupVgg(exec, sdkUrl);

  VggDepContainer<std::shared_ptr<VggExec>>::get() = exec;
  VggDepContainer<std::shared_ptr<VggSdk>>::get().reset(new VggSdk);
}

void MainComposer::teardown()
{
  VggDepContainer<std::shared_ptr<VggExec>>::get().reset();
  VggDepContainer<std::shared_ptr<VggSdk>>::get().reset();
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