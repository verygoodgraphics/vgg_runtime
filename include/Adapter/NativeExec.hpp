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

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include "Domain/Event.hpp"
#include "Domain/IVggEnv.hpp"
#include "Domain/VggJSEngine.hpp"
namespace VGG
{
class NativeExecImpl;
}
namespace node
{
class Environment;
}

namespace VGG
{

class NativeExec final : public VggJSEngine
{
public:
  static std::shared_ptr<NativeExec> sharedInstance();
  ~NativeExec();

  bool evalScript(const std::string& code) override;
  bool evalModule(const std::string& code) override;
  bool evalModule(const std::string& code, VGG::EventPtr event, std::shared_ptr<IVggEnv> env)
    override;

  void openUrl(const std::string& url, const std::string& target) override;

  using InjectFn = std::function<void(node::Environment*)>;
  bool inject(InjectFn fn);

  void release();

private:
  std::shared_ptr<std::thread>    m_thread;
  std::shared_ptr<NativeExecImpl> m_impl;

  NativeExec();

  void teardown();
};

} // namespace VGG
