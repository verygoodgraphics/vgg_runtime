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
#include <string>
#include "Event.hpp"
#include "IVggEnv.hpp"
#include "VggJSEngine.hpp"

namespace VGG
{

class VggExec
{
public:
  VggExec(const std::shared_ptr<VggJSEngine>& jsEngine, const std::shared_ptr<IVggEnv>& env)
    : m_jsEngine(jsEngine)
    , m_env(env)
  {
  }
  ~VggExec() = default;

  bool evalScript(const std::string& script);
  bool evalModule(const std::string& script);
  bool evalModule(const std::string& code, VGG::EventPtr event);

  void openUrl(const std::string& url, const std::string& target);

private:
  std::shared_ptr<VggJSEngine> m_jsEngine;
  std::shared_ptr<IVggEnv>     m_env;

  void setEnv();
};

} // namespace VGG
