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
#ifndef VGG_JS_ENGINE_HPP
#define VGG_JS_ENGINE_HPP

#include "Domain/Event.hpp"
#include "Domain/IVggEnv.hpp"

#include <memory>
#include <string_view>

namespace VGG
{

class VggJSEngine
{
public:
  virtual ~VggJSEngine() = default;

  virtual bool evalScript(const std::string& code) = 0;
  virtual bool evalModule(const std::string& code) = 0;
  virtual bool evalModule(
    const std::string&       code,
    VGG::EventPtr            event,
    std::shared_ptr<IVggEnv> env) = 0;

  virtual void openUrl(const std::string& url, const std::string& target) = 0;
};

} // namespace VGG
#endif
