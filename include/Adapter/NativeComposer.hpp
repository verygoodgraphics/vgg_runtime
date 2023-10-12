/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Application/PlatformComposer.hpp"

#include "Application/VggSdk.hpp"
#include "NativeExec.hpp"
#include "VggSdkAddon.hpp"

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
