#pragma once

#include "Main/PlatformComposer.hpp"

#include "Sdk/VggSdk.hpp"
#include "PlatformAdapter/Native/Sdk/VggSdkAddon.hpp"
#include "PlatformAdapter/Native/Exec/NativeExec.hpp"

#include <string>
#include <memory>

class NativeComposer : public PlatformComposer
{
  std::string m_sdkUrl;
  bool m_catchJsException;
  std::shared_ptr<NativeExec> m_native_exec;

public:
  NativeComposer(const std::string& sdkUrl, bool catchJsException = true)
    : PlatformComposer()
    , m_sdkUrl{ sdkUrl }
    , m_catchJsException{ catchJsException }
    , m_native_exec{ std::make_shared<NativeExec>() }
  {
  }

  virtual std::shared_ptr<VggJSEngine> createJsEngine() override
  {
    return m_native_exec;
  }

  virtual void platformSetup() override
  {
    m_native_exec->inject([](node::Environment* env) { link_vgg_sdk_addon(env); });
    setupVgg();

    VGG::DIContainer<std::shared_ptr<VggSdk>>::get().reset(new VggSdk);
  }

  virtual void platformTeardown() override
  {
    VGG::DIContainer<std::shared_ptr<VggSdk>>::get().reset();
  }

private:
  void setupVgg()
  {
    std::string set_vgg_code(R"(
      const { getVgg, getVggSdk, setVgg } = await import(")");
    set_vgg_code.append(m_sdkUrl);
    set_vgg_code.append(R"(");
      var vggSdkAddon = process._linkedBinding('vgg_sdk_addon');
      await setVgg(vggSdkAddon);
    )");
    m_native_exec->evalModule(set_vgg_code);

    if (m_catchJsException)
    {
      std::string catch_exception{ R"(
        const __vggErrorHandler = (err) => {
          console.error(err)
        }
        process.on('uncaughtException', __vggErrorHandler)
      )" };
      m_native_exec->evalScript(catch_exception);
    }
  }
};
