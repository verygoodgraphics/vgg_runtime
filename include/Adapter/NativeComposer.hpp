/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include <memory>
#include "Application/PlatformComposer.hpp"
#include "NativeExec.hpp"
#include "VggSdkAddon.hpp"
namespace VGG
{
class VggJSEngine;
}
namespace node
{
class Environment;
}

namespace VGG
{

class NativeComposer : public PlatformComposer
{
  bool                        m_catchJsException;
  std::shared_ptr<NativeExec> m_native_exec;

public:
  NativeComposer(bool catchJsException = true)
    : PlatformComposer()
    , m_catchJsException{ catchJsException }
    , m_native_exec{ NativeExec::sharedInstance() }
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
  }

  virtual void platformTeardown() override
  {
  }

private:
  void setupVgg();
};

} // namespace VGG